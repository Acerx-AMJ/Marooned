#include "world.h"
#include "player.h"
#include "raymath.h"
#include "resources.h"

bool controlPlayer = false;
unsigned char* heightmapPixels = nullptr;
Player player = {};

Vector3 boatPosition = {6000, -20, 0.0};
float boatSpeed = 200;
float waterHeightY = 60;


const float TREE_HEIGHT_RATIO = 0.80f;
const float BUSH_HEIGHT_RATIO = 0.80f;

std::vector<Character> raptors;

std::vector<Character*> raptorPtrs;

std::vector<Bullet> activeBullets;

void removeAllRaptors(){
    raptorPtrs.clear();
    raptors.clear();

}


void generateRaptors(int amount, Vector3 centerPos, float radius) {
    raptors.clear();
    raptorPtrs.clear();

    int spawned = 0;
    int attempts = 0;
    const int maxAttempts = 1000; // More attempts since spawnable area may be smaller

    while (spawned < amount && attempts < maxAttempts) {
        ++attempts;

        // Random position around centerPos within a circle (not square)
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float distance = GetRandomValue(500, (int)radius); // Optional: minimum distance
        float x = centerPos.x + cosf(angle) * distance;
        float z = centerPos.z + sinf(angle) * distance;

        Vector3 spawnPos = { x, 0.0f, z };
        float terrainHeight = GetHeightAtWorldPosition(spawnPos, heightmap, terrainScale);

        if (terrainHeight > 60.0f) {
            float spriteHeight = 200 * 0.5f;
            spawnPos.y = terrainHeight + spriteHeight / 2.0f;

            Character raptor(spawnPos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f);
            raptors.push_back(raptor);
            ++spawned;
        }
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


