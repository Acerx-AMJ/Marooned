#pragma once
#include "raylib.h"
#include <string>


extern RenderTexture2D sceneTexture, postProcessTexture, depthEffectTexture;
extern Texture2D bushTex, shadowTex, raptorTexture, gunTexture, muzzleFlash, backDrop, smokeSheet, bloodSheet, skeletonSheet, doorTexture, wallFallback,
healthPotTexture, keyTexture, swordBloody, swordClean, fireSheet, pirateSheet, coinTexture, spiderSheet, spiderWebTexture, brokeWebTexture, explosionSheet;
extern Shader fogShader, skyShader, waterShader, terrainShader, shadowShader, simpleFogShader, bloomShader, depthShader, pbrShader;
extern Model terrainModel, skyModel, waterModel, shadowQuad, palmTree, palm2, bush, boatModel, floorTile2,floorTile3,chestModel, fireballModel, 
bottomPlane, blunderbuss, floorTile, doorWay, wall, barrelModel, pillarModel, swordModel, lampModel, brokeBarrel, staffModel;
extern Image heightmap;
extern Mesh terrainMesh;
extern Vector3 terrainScale;

extern int sceneTextureLoc;
extern int sceneDepthLoc; 

extern Vector2 screenResolution;

void UpdateShaders(Camera& camera);
void LoadAllResources();
void UnloadAllResources();

