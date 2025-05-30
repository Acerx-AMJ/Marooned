#pragma once
#include "raylib.h"
#include <string>

extern RenderTexture2D sceneTexture;
extern Texture2D bushTex, shadowTex, raptorFront, raptorTexture;
extern Shader fogShader, skyShader, waterShader, terrainShader, shadowShader;
extern Model terrainModel, skyModel, waterModel, shadowQuad, palmTree, palm2, bush, boatModel;
extern Image heightmap;
extern Mesh terrainMesh;
extern Vector3 terrainScale;

extern Vector2 screenResolution;

void LoadAllResources();
void UnloadAllResources();
