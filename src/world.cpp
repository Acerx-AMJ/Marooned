#include "world.h"
#include "player.h"
#include "raymath.h"
#include "resourceManager.h"
#include "vegetation.h"
#include "dungeonGeneration.h"
#include "boat.h"
#include "algorithm"
#include "sound_manager.h"
#include "rlgl.h"
#include "pathfinding.h"



GameState currentGameState = GameState::Menu;
//global variables, clean these up somehow. 

Model terrainModel;
Image heightmap;
Mesh terrainMesh;
Vector3 terrainScale = {16000.0f, 200.0f, 16000.0f}; //very large x and z, 

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

float waterHeightY = 60;
Vector3 bottomPos = {0, waterHeightY - 100, 0};
float dungeonPlayerHeight = 100;
float ceilingHeight = 400;
float fadeToBlack = 0.0f;
float vignetteIntensity = 0.0f;
float vignetteFade = 0.0f;
float vignetteStrengthValue = 0.2;
float bloomStrengthValue = 0.0;
bool isFading = false;
float fadeSpeed = 1.0f; // units per second
bool fadeIn = true; 
float tileSize = 200;

int selectedOption = 0;
float floorHeight = 80;
float wallHeight = 270;
float dungeonEnemyHeight = 165;
float ElapsedTime = 0.0f;
bool debugInfo = false;
bool isLoadingLevel = false;

std::vector<Bullet> activeBullets;
std::vector<Decal> decals;
std::vector<MuzzleFlash> activeMuzzleFlashes;
std::vector<Collectable> collectables;
std::vector<CollectableWeapon> worldWeapons;
std::vector<Character> enemies;  
std::vector<Character*> enemyPtrs;
std::vector<DungeonEntrance> dungeonEntrances;


void BeginCustom3D(Camera3D camera, float farClip) {
    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    float nearClip = 60.0f; //20 wider than the capsule. to 50k outside, 10k in dungeons

    float testFOV = 45.0f; // 45 is default 
    Matrix proj = MatrixPerspective(DEG2RAD * camera.fovy, (float)GetScreenWidth() / (float)GetScreenHeight(), nearClip, farClip);

    rlMultMatrixf(MatrixToFloat(proj));
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(view));
}


void InitLevel(const LevelData& level, Camera camera) {
    isLoadingLevel = true;
    //Called when starting game and changing level. init the level you pass it. the level is chosen by menu or door's linkedLevelIndex. 
    ClearLevel();//clears everything. 
    camera.position = player.position; //start as player, not freecam.
    levelIndex = level.levelIndex; //update current level index to new level. 

    //we still generate terrain mesh for dungeons.


    vignetteStrengthValue = 0.2f; //less of vignette outdoors.
    bloomStrengthValue = 0.0f; //turn on bloom in dungeons
    SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);
    
    // Load and format the heightmap image
    heightmap = LoadImage(level.heightmapPath.c_str());
    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    
    terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
    terrainModel = LoadModelFromMesh(terrainMesh);
    terrainModel.materials[0].shader = R.GetShader("terrainShader");
    dungeonEntrances = level.entrances; //get level entrances from level data

    generateRaptors(level.raptorCount, level.raptorSpawnCenter, 6000);
    GenerateEntrances();
    generateVegetation(); 
    if (!level.isDungeon) InitBoat(player_boat, boatPosition);
    

   
    if (level.isDungeon){
        isDungeon = true;
        vignetteStrengthValue = 0.5f; //darker vignette in dungeons
        bloomStrengthValue = 0.35f; //turn on bloom in dungeons
        SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
        SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);


        LoadDungeonLayout(level.dungeonPath);
        ConvertImageToWalkableGrid(dungeonImg);
        GenerateFloorTiles(floorHeight);//80
        GenerateWallTiles(wallHeight); //model is 400 tall with origin at it's center, so wallHeight is floorHeight + model height/2. 270
        GenerateCeilingTiles(ceilingHeight);//400
        GenerateBarrels(floorHeight);
        GenerateSpiderWebs(floorHeight);
        GenerateChests(floorHeight);
        GeneratePotions(floorHeight);
        GenerateKeys(floorHeight);
        GenerateWeapons(200);
        GenerateLightSources(floorHeight);
        GenerateDoorways(floorHeight, levelIndex); //calls generate doors from archways
        //generate enemies.
        GenerateSkeletonsFromImage(dungeonEnemyHeight); //165
        GeneratePiratesFromImage(dungeonEnemyHeight);
        GenerateSpiderFromImage(dungeonEnemyHeight);

        if (levelIndex == 4) levels[0].startPosition = {-5653, 200, 6073}; //exit dungeon 3 to dungeon enterance 2 position. 
        
      
    }
    
    
    Vector3 resolvedSpawn = ResolveSpawnPoint(level, isDungeon, first, floorHeight);

    InitPlayer(player, resolvedSpawn); //start at green pixel if there is one. otherwise level.startPos or first startPos
    isLoadingLevel = false;
    ResetAllBakedTints();
    BakeStaticLighting();

    //start with blunderbus and sword in that order
    player.collectedWeapons = {WeaponType::Blunderbuss, WeaponType::Sword};
    player.activeWeapon = WeaponType::Blunderbuss;
    player.currentWeaponIndex = 0;

}

void UpdateFade(float deltaTime, Camera& camera){
    //fades out on death, and level transition if pendingLevelIndex != -1
    if (isFading) {
        if (fadeIn) {
            fadeToBlack += fadeSpeed * deltaTime;
            if (fadeToBlack >= 1.0f) {
                fadeToBlack = 1.0f;
                isFading = false;

                if (pendingLevelIndex != -1) {
                    InitLevel(levels[pendingLevelIndex], camera); //Start new Level
                    pendingLevelIndex = -1;

                    // Start fading back in
                    fadeIn = false;
                    isFading = true;
                }
            }
        } else {
            fadeToBlack -= fadeSpeed * deltaTime;
            if (fadeToBlack <= 0.0f) {
                fadeToBlack = 0.0f;
                isFading = false;
            }
        }
        Shader fogShader = R.GetShader("fogShader");
        SetShaderValue(fogShader, GetShaderLocation(fogShader, "fadeToBlack"), &fadeToBlack, SHADER_UNIFORM_FLOAT);
    }


}


void removeAllCharacters(){
    enemies.clear();
    enemyPtrs.clear();

}

void GenerateEntrances(){
    for (const DungeonEntrance& e : dungeonEntrances) {
        Door d;
        d.position = e.position;
        d.rotationY = 0.0f; 
        d.doorTexture = R.GetTexture("doorTexture");
        d.isOpen = false;
        d.isLocked = false;
        d.scale = {300, 365, 1};
        d.tint = WHITE;
        d.linkedLevelIndex = e.linkedLevelIndex; //use entrance's linkedLevelIndex to determine which dungeon to enter from overworld
        //allowing more than one enterance to dungeon per overworld

        d.doorType = DoorType::GoToNext; //Go to Dungeon

        float halfWidth = 200.0f;   // Half of the 400-unit wide doorway
        float height = 365.0f;
        float depth = 20.0f;        // Thickness into the doorway (forward axis)

        d.collider = MakeDoorBoundingBox(d.position, d.rotationY, halfWidth, height, depth); //the whole archway is covered by collider

        doors.push_back(d);

        DoorwayInstance dw;
        dw.position = e.position;
        dw.rotationY = 90.0f * DEG2RAD; //rotate to match door 0 rotation, we could rotate door to match arch instead.
        dw.isOpen = false;
        dw.isLocked = false;
        dw.tint = GRAY;

        doorways.push_back(dw);
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

        Character raptor(spawnPos, R.GetTexture("raptorTexture"), 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);
        enemies.push_back(raptor);
        enemyPtrs.push_back(&enemies.back()); 
        ++spawned;
    }


}

void UpdateEnemies(float deltaTime) {
    if (isLoadingLevel) return;
    for (Character& e : enemies){
        e.Update(deltaTime, player, heightmap, terrainScale);
    }
}

void UpdateMuzzleFlashes(float deltaTime) {
    for (auto& flash : activeMuzzleFlashes) {
        flash.age += deltaTime;
    }

    // Remove expired flashes
    activeMuzzleFlashes.erase(
        std::remove_if(activeMuzzleFlashes.begin(), activeMuzzleFlashes.end(),
                       [](const MuzzleFlash& flash) { return flash.age >= flash.lifetime; }),
        activeMuzzleFlashes.end());
}


void UpdateBullets(Camera& camera, float deltaTime) {
    for (Bullet& b : activeBullets) {
        b.Update(camera, deltaTime);
    }

    activeBullets.erase( //erase dead bullets. 
        std::remove_if(activeBullets.begin(), activeBullets.end(),
                    [](const Bullet& b) { return !b.IsAlive(); }),
        activeBullets.end());

}

void UpdateCollectables(Camera& camera, float deltaTime) { 
    for (int i = 0; i < collectables.size(); i++) {
        collectables[i].Update(deltaTime);

        // Pickup logic
        if (collectables[i].CheckPickup(player.position, 180.0f)) { //180 radius
            if (collectables[i].type == CollectableType::HealthPotion) {
                player.inventory.AddItem("HealthPotion");
                SoundManager::GetInstance().Play("clink");
            }
            else if (collectables[i].type == CollectableType::Key) {
                player.inventory.AddItem("GoldKey");
                SoundManager::GetInstance().Play("key");
            }
            else if (collectables[i].type == CollectableType::Gold) {
                player.gold += collectables[i].value;
                SoundManager::GetInstance().Play("key");
            } else if (collectables[i].type == CollectableType::ManaPotion) {
                player.inventory.AddItem("ManaPotion");
                SoundManager::GetInstance().Play("clink");
            }

            collectables.erase(collectables.begin() + i);
            i--;
        }
    }
}




void UpdateDecals(float deltaTime){
    for (auto& d : decals) {
        d.Update(deltaTime);
        
    }
    decals.erase(std::remove_if(decals.begin(), decals.end(),
                [](const Decal& d) { return !d.alive; }),
                decals.end());
}

void lightBullets(float deltaTime){
    //Fireball/iceball light

   // === Clean up expired bullet lights ===
    for (int i = bulletLights.size() - 1; i >= 0; --i) {
        bulletLights[i].age += deltaTime;
        if (bulletLights[i].age >= bulletLights[i].lifeTime) {
            bulletLights.erase(bulletLights.begin() + i);
        }
    }

   // === Collect new lights to add after cleanup ===
    std::vector<LightSource> newBulletLights;
    for (const Bullet& bullet : activeBullets) {
        if (bullet.type == BulletType::Default) continue;
        LightSource bulletLight;
        bulletLight.position = bullet.GetPosition();
        bulletLight.range = 300.0f;
        //bulletLight.intensity = 1.2f;
        bulletLight.lifeTime = 0.5f;
        bulletLight.age = 0.0f;
        if (bullet.type == BulletType::Iceball) {
            bulletLight.colorTint = {0.0f, 0.7f, 0.9f};
            bulletLight.type = LightType::Iceball;
        } else if (bullet.type == BulletType::Fireball) {
            bulletLight.colorTint = {1.0f, 0.15f, 0.0f};
            bulletLight.type = LightType::Fireball;
        } else {
            bulletLight.type = LightType::Other;
        }

        newBulletLights.push_back(bulletLight);
    }

    //=== Now safely append them ===
    bulletLights.insert(bulletLights.end(), newBulletLights.begin(), newBulletLights.end());
}


void DrawBloodParticles(Camera& camera){
    for (Character* enemy : enemyPtrs) { //draw enemy blood, blood is 3d so draw before billboards. 
            enemy->bloodEmitter.Draw(camera);
    }
}

void DrawBullets(Camera& camera) {
    for (const Bullet& b : activeBullets) {
        if (b.IsAlive()){
             b.Draw(camera);
        }
    }

}


Vector3 ResolveSpawnPoint(const LevelData& level, bool isDungeon, bool first, float floorHeight) 
{
    Vector3 resolvedSpawn = level.startPosition; // default fallback

    if (first) {
        resolvedSpawn = {5475.0f, 300.0f, -5665.0f}; // hardcoded spawn for first level
    }

    if (isDungeon) {
        Vector3 pixelSpawn = FindSpawnPoint(dungeonPixels, dungeonWidth, dungeonHeight, tileSize, floorHeight);
        if (pixelSpawn.x != 0 || pixelSpawn.z != 0) {
            resolvedSpawn = pixelSpawn; // green pixel overrides everything
        }
    }

    return resolvedSpawn;
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
    //please refactor this, this is from when we only had a camera. 
    if (controlPlayer) {
        UpdatePlayer(player, GetFrameTime(), camera);
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

void ClearLevel() {
    billboardRequests.clear();
    removeAllCharacters();\
    activeBullets.clear();
    ClearDungeon();
    bulletLights.clear();
    dungeonEntrances.clear();

    RemoveAllVegetation();

    if (terrainMesh.vertexCount > 0) UnloadMesh(terrainMesh); //unload mesh when switching levels. 
    if (heightmap.data != nullptr) UnloadImage(heightmap); 

    isDungeon = false;
}



