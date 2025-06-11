#include "world.h"
#include "player.h"
#include "raymath.h"
#include "resources.h"
#include "vegetation.h"
#include "dungeonGeneration.h"



GameState currentGameState = GameState::Menu;
int levelIndex = 0;
bool controlPlayer = false;
bool isDungeon = false;
unsigned char* heightmapPixels = nullptr;
Player player = {};
Vector3 startPosition = {5475.0f, 300.0f, -5665.0f}; //middle island start pos
Vector3 boatPosition = {6000, -20, 0.0};
Vector3 playerSpawnPoint = {0,0,0};
float boatSpeed = 200;
float waterHeightY = 60;
float dungeonPlayerHeight = 100;
float fadeToBlack = 0.0f;
float vignetteIntensity = 0.0f;
float vignetteFade = 0.0f;
const float TREE_HEIGHT_RATIO = 0.80f;
const float BUSH_HEIGHT_RATIO = 0.80f;
int selectedOption = 0;
float floorHeight = 80;
std::vector<Character> raptors;

std::vector<Character*> raptorPtrs;

std::vector<Bullet> activeBullets;
std::vector<Decal> decals;



void removeAllRaptors(){
    raptorPtrs.clear();
    raptors.clear();

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
    raptors.clear();
    raptorPtrs.clear();

    int spawned = 0;
    int attempts = 0;
    const int maxAttempts = 1000;

    while (spawned < amount && attempts < maxAttempts) {
        ++attempts;

        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float distance = GetRandomValue(500, (int)radius);
        float x = centerPos.x + cosf(angle) * distance;
        float z = centerPos.z + sinf(angle) * distance;

        Vector3 spawnPos = { x, 0.0f, z };
        
        if (isDungeon) {
            // Convert world x,z to dungeon tile coordinates
            const float tileSize = 200.0f; 
            int gridX = (int)(x / tileSize);
            int gridY = (int)(z / tileSize);

            if (!IsDungeonFloorTile(gridX, gridY)) continue;

            float dh = 85.0f;
            float spriteHeight = 200 * 0.5f;
            spawnPos.y = dh + spriteHeight / 2.0f;

        } else {
            float terrainHeight = GetHeightAtWorldPosition(spawnPos, heightmap, terrainScale);
            if (terrainHeight <= 80.0f) continue;

            float spriteHeight = 200 * 0.5f;
            spawnPos.y = terrainHeight + spriteHeight / 2.0f;
        }

        //std::cout << "generated raptor\n";

        Character raptor(spawnPos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f);
        raptors.push_back(raptor);
        ++spawned;
    }

    for (Character& r : raptors) {
        raptorPtrs.push_back(&r);
    }
}


void regenerateRaptors(int amount, Vector3 position, float radius){
    generateRaptors(amount, player.position, 6000); 
}


float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale) {
    //read heightmap data to determine height in world space. 
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


