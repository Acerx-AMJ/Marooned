#pragma once
#include "raylib.h"
#include <string>

extern RenderTexture2D sceneTexture;
extern Texture2D bushTex, shadowTex;
extern Shader fogShader, skyShader, waterShader, terrainShader, shadowShader;
extern Model terrainModel, skyModel, waterModel, shadowQuad, palmTree, palm2, bush, boat;
extern Image heightmap;
extern Mesh terrainMesh;
extern Vector3 terrainScale;
extern float waterHeightY;
extern Vector2 screenResolution;

void LoadAllResources();
void UnloadAllResources();
