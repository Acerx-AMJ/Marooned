#include "world.h"
#include "player.h"

bool controlPlayer = false;
unsigned char* heightmapPixels = nullptr;
Player player = {};

Vector3 boatPosition = {3000, -20, -0.0};
float boatSpeed = 200;
float waterHeightY = 60;

const float TREE_HEIGHT_RATIO = 0.80f;
const float BUSH_HEIGHT_RATIO = 0.80f;
