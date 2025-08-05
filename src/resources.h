#pragma once
#include "raylib.h"
#include <string>


extern Model terrainModel;
extern Image heightmap;
extern Mesh terrainMesh;
extern Vector3 terrainScale;

// extern int sceneTextureLoc;
// extern int sceneDepthLoc; 

extern Vector2 screenResolution;

void UpdateShaders(Camera& camera);
void LoadAllResources();
void UnloadAllResources();

