#pragma once
#include "raylib.h"
#include <string>


extern RenderTexture2D sceneTexture;
extern Texture2D bushTex, shadowTex, raptorTexture, gunTexture, muzzleFlash, backDrop, smokeSheet, bloodSheet, skeletonSheet, doorTexture;
extern Shader fogShader, skyShader, waterShader, terrainShader, shadowShader;
extern Model terrainModel, skyModel, waterModel, shadowQuad, palmTree, palm2, bush, boatModel, 
gunModel, bottomPlane, blunderbuss, floorTile, doorWay, wall, barrelModel, pillarModel;
extern Image heightmap;
extern Mesh terrainMesh;
extern Vector3 terrainScale;

extern Vector2 screenResolution;
//extern int lightPosLoc;
//extern int camPosLocD;
void UpdateShaders(Camera& camera);
void LoadAllResources();
void UnloadAllResources();

