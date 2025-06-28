#include "world.h"
#include "player.h"
#include "raymath.h"
#include "resources.h"
#include "vegetation.h"
#include "dungeonGeneration.h"
#include "boat.h"



GameState currentGameState = GameState::Menu;
//global variables, should probably be put into a struct or something. 

int levelIndex = 0; //current level, levels[levelIndex]
int previousLevelIndex = 0;
bool first = true; //for first player start position
bool controlPlayer = false;
bool isDungeon = false;
unsigned char* heightmapPixels = nullptr;
Player player = {};
Vector3 startPosition = {5475.0f, 300.0f, -5665.0f}; //middle island start pos
Vector3 boatPosition = {6000, -20, 0.0};
Vector3 playerSpawnPoint = {0,0,0};
int pendingLevelIndex = -1; //wait to switch level until faded out. UpdateFade() needs to know the next level index. 
//float boatSpeed = 200;
float waterHeightY = 60;
Vector3 bottomPos = {0, waterHeightY - 100, 0};
float dungeonPlayerHeight = 100;
float ceilingHeight = 400;
float fadeToBlack = 0.0f;
float vignetteIntensity = 0.0f;
float vignetteFade = 0.0f;
float vignetteStrengthValue = 0.2;
bool isFading = false;
float fadeSpeed = 1.0f; // units per second
bool fadeIn = true; 
float tileSize = 200;
const float TREE_HEIGHT_RATIO = 0.80f;
const float BUSH_HEIGHT_RATIO = 0.90f;
int selectedOption = 0;
float floorHeight = 80;
float wallHeight = 270;
float dungeonEnemyHeight = 165;
float ElapsedTime = 0.0f;
bool debugInfo = true;

// std::vector<Character> raptors;
// std::vector<Character*> raptorPtrs;

// std::vector<Character> skeletons;
// std::vector<Character*> skeletonPtrs;

// std::vector<Character> pirates;
// std::vector<Character*> piratePtrs;

std::vector<Bullet> activeBullets;
std::vector<Decal> decals;

std::vector<DungeonEntrance> dungeonEntrances;

std::vector<Collectable> collectables;

std::vector<Character> enemies;  
std::vector<Character*> enemyPtrs;



void removeAllCharacters(){
    enemies.clear();
    enemyPtrs.clear();

}

bool CheckTreeCollision(const TreeInstance& tree, const Vector3& playerPos) {
    Vector3 treeBase = {
        tree.position.x + tree.xOffset,
        tree.position.y + tree.yOffset,
        tree.position.z + tree.zOffset
    };

    float dx = playerPos.x - treeBase.x;
    float dz = playerPos.z - treeBase.z;
    float horizontalDistSq = dx * dx + dz * dz;

    if (horizontalDistSq < tree.colliderRadius * tree.colliderRadius &&
        playerPos.y >= treeBase.y &&
        playerPos.y <= treeBase.y + tree.colliderHeight) {
        return true;
    }

    return false;
}

void ResolveTreeCollision(const TreeInstance& tree, Vector3& playerPos) {
    Vector3 treeBase = {
        tree.position.x + tree.xOffset,
        tree.position.y + tree.yOffset,
        tree.position.z + tree.zOffset
    };

    float dx = playerPos.x - treeBase.x;
    float dz = playerPos.z - treeBase.z;
    float distSq = dx * dx + dz * dz;

    float radius = tree.colliderRadius;
    if (distSq < radius * radius) {
        float dist = sqrtf(distSq);
        float overlap = radius - dist;

        if (dist > 0.01f) {
            dx /= dist;
            dz /= dist;
            playerPos.x += dx * overlap;
            playerPos.z += dz * overlap;
        }
    }
}




void generateRaptors(int amount, Vector3 centerPos, float radius) {

    int spawned = 0;
    int attempts = 0;
    const int maxAttempts = 1000;
    //try 1000 times to spawn all the raptors either above 80 on heightmap or on empty floor tiles in dungeon. 
    while (spawned < amount && attempts < maxAttempts) {
        ++attempts;

        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float distance = GetRandomValue(500, (int)radius);
        float x = centerPos.x + cosf(angle) * distance;
        float z = centerPos.z + sinf(angle) * distance;

        Vector3 spawnPos = { x, 0.0f, z }; //random x, z  get height diferrently for dungeon
        
        if (isDungeon) {
            // Convert world x,z to dungeon tile coordinates
            const float tileSize = 200.0f; 
            int gridX = (int)(x / tileSize);
            int gridY = (int)(z / tileSize);

            if (!IsDungeonFloorTile(gridX, gridY)) continue; //try again

            float dh = 85.0f;
            float spriteHeight = 200 * 0.5f;
            spawnPos.y = dh + spriteHeight / 2.0f;

        } else {
            float terrainHeight = GetHeightAtWorldPosition(spawnPos, heightmap, terrainScale);
            if (terrainHeight <= 80.0f) continue; //try again

            float spriteHeight = 200 * 0.5f;
            spawnPos.y = terrainHeight + spriteHeight / 2.0f;
        }

        //std::cout << "generated raptor\n";

        Character raptor(spawnPos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);
        enemies.push_back(raptor);
        enemyPtrs.push_back(&enemies.back()); 
        ++spawned;
    }


}


void regenerateRaptors(int amount, Vector3 position, float radius){
    generateRaptors(amount, player.position, 6000); //regenerate raptors when pressing O
}


float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale) {
    //read heightmap pixels and return the height in world space. 
    int width = heightmap.width;
    int height = heightmap.height;
    unsigned char* pixels = (unsigned char*)heightmap.data;

    // Convert world X/Z into heightmap image coordinates
    float xPercent = (position.x + terrainScale.x / 2.0f) / terrainScale.x;
    float zPercent = (position.z + terrainScale.z / 2.0f) / terrainScale.z;

    // Clamp to valid range
    xPercent = Clamp(xPercent, 0.0f, 1.0f);
    zPercent = Clamp(zPercent, 0.0f, 1.0f);

    // Convert to pixel indices
    int x = (int)(xPercent * (width - 1));
    int z = (int)(zPercent * (height - 1));
    int index = z * width + x;

    // Get grayscale pixel and scale to world height
    float heightValue = (float)pixels[index] / 255.0f;
    return heightValue * terrainScale.y;
}


void UpdateCustomCamera(Camera3D* camera, float deltaTime) {
    //Free camera control
    static float yaw = 0.0f;
    static float pitch = 0.0f;

    float moveSpeed = 1000.0f * deltaTime;
    float lookSensitivityMouse = 0.03f;
    float lookSensitivityGamepad = 100.0f * deltaTime;

    // Get input from both controller and mouse
    Vector2 mouseDelta = GetMouseDelta();
    yaw   += mouseDelta.x * lookSensitivityMouse;
    pitch += -mouseDelta.y * lookSensitivityMouse;


    if (IsGamepadAvailable(0)) {
        float rightX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        float rightY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        yaw   += rightX * lookSensitivityGamepad;
        pitch += -rightY * lookSensitivityGamepad;
    }

    // Clamp pitch to avoid flipping
    pitch = Clamp(pitch, -89.0f, 89.0f);

    // Direction vector based on yaw/pitch
    Vector3 forward = {
        cosf(pitch * DEG2RAD) * cosf(yaw * DEG2RAD),
        sinf(pitch * DEG2RAD),
        cosf(pitch * DEG2RAD) * sinf(yaw * DEG2RAD)
    };
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera->up));

    // Combined movement input (keyboard + gamepad left stick)
    float inputX = 0.0f;
    float inputZ = 0.0f;
    float inputY = 0.0f;

    if (IsKeyDown(KEY_W)) inputZ += 1.0f;
    if (IsKeyDown(KEY_S)) inputZ -= 1.0f;
    if (IsKeyDown(KEY_A)) inputX -= 1.0f;
    if (IsKeyDown(KEY_D)) inputX += 1.0f;
    if (IsKeyDown(KEY_SPACE)) inputY += 1.0f; //fly up and down
    if (IsKeyDown(KEY_LEFT_SHIFT)) inputY -= 1.0f;

    if (IsGamepadAvailable(0)) {
        inputX += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        inputZ += -GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y); // Y axis is inverted
    }

    // Apply movement
    Vector3 movement = Vector3Zero();
    movement = Vector3Add(movement, Vector3Scale(right, inputX * moveSpeed));
    movement = Vector3Add(movement, Vector3Scale(forward, inputZ * moveSpeed));
    movement.y += inputY * moveSpeed;

    camera->position = Vector3Add(camera->position, movement);
    camera->target = Vector3Add(camera->position, forward);
}

void UpdateCameraAndPlayer(Camera& camera, Player& player, bool controlPlayer, float deltaTime) {
    if (controlPlayer) {
        UpdatePlayer(player, GetFrameTime(), terrainMesh, camera);
        DisableCursor();

        float yawRad = DEG2RAD * player.rotation.y;
        float pitchRad = DEG2RAD * player.rotation.x;

        Vector3 forward = {
            cosf(pitchRad) * sinf(yawRad),
            sinf(pitchRad),
            cosf(pitchRad) * cosf(yawRad)
        };

        float camYOffset = player.isSwimming ? -40.0f : 1.5f;  // swimming lowers the camera
        camera.position = Vector3Add(player.position, (Vector3){ 0, camYOffset, 0 });
        //camera.position = Vector3Add(player.position, (Vector3){ 0, 1.5f, 0 });
        camera.target = Vector3Add(camera.position, forward);


    } else {
        UpdateCustomCamera(&camera, deltaTime);
        if (player.onBoard) {
            player.position = Vector3Add(player_boat.position, {0, 200.0f, 0});
        }
    }
}


void HandleCameraPlayerToggle(Camera& camera, Player& player, bool& controlPlayer) {
    static Vector3 savedForward;

    if (IsKeyPressed(KEY_TAB)) {
        controlPlayer = !controlPlayer;

        if (controlPlayer) {
            savedForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            float yaw = atan2f(savedForward.x, savedForward.z) * RAD2DEG;
            float pitch = -asinf(savedForward.y) * RAD2DEG;

            player.position = camera.position;
            player.velocity = { 0 };
            player.rotation.y = yaw;
            player.rotation.x = Clamp(pitch, -89.0f, 89.0f);
        } else {
            camera.position = Vector3Add(player.position, { 0, 1.5f, 0 });
            camera.target = Vector3Add(camera.position, savedForward);
        }
    }

}



