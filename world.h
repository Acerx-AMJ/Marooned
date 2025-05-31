// world.h
#pragma once
#include "raylib.h"
#include "player.h"
#include <vector>
#include "character.h"

extern bool controlPlayer;
//extern Image heightmap;
extern unsigned char* heightmapPixels;
//extern Vector3 terrainScale;
//extern Mesh terrainMesh;
extern Player player;
extern Vector3 boatPosition;
extern float boatSpeed;
extern float waterHeightY;
extern const float TREE_HEIGHT_RATIO;
extern const float BUSH_HEIGHT_RATIO;

extern std::vector<Character> raptors;
extern std::vector<Character*> raptorPtrs;


float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale);
void removeAllRaptors();
void generateRaptors(int amount, Vector3 centerPos, float radius);
void regenerateRaptors(int amount, Vector3 centerPos, float radius);

