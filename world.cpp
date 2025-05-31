#include "world.h"
#include "player.h"
#include "raymath.h"

bool controlPlayer = false;
unsigned char* heightmapPixels = nullptr;
Player player = {};

Vector3 boatPosition = {3000, -20, -0.0};
float boatSpeed = 200;
float waterHeightY = 60;

const float TREE_HEIGHT_RATIO = 0.80f;
const float BUSH_HEIGHT_RATIO = 0.80f;

std::vector<Character> raptors;

std::vector<Character*> raptorPtrs;

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


