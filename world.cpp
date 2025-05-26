#include "world.h"
#include "player.h"

bool controlPlayer = false;
Image heightmap = { 0 };
unsigned char* heightmapPixels = nullptr;
Vector3 terrainScale = { 0 };
Mesh terrainMesh = { 0 };
Player player = {};

const float TREE_HEIGHT_RATIO = 0.80f;
const float BUSH_HEIGHT_RATIO = 0.80f;
