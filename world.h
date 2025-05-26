// world.h
#pragma once
#include "raylib.h"
#include "player.h"


extern bool controlPlayer;
extern Image heightmap;
extern unsigned char* heightmapPixels;
extern Vector3 terrainScale;
extern Mesh terrainMesh;
extern Player player;

extern const float TREE_HEIGHT_RATIO;
extern const float BUSH_HEIGHT_RATIO;


