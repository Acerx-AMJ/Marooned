#pragma once

#include "raylib.h"
#include "vector"
#include "dungeonGeneration.h"

struct XZBounds { float minX, maxX, minZ, maxZ; };

struct BakedLightmap {
    Texture2D tex = {0};
    int w = 0, h = 0;
    // World-space mapping (XZ -> [0..1])
    float minX = 0, minZ = 0, sizeX = 1, sizeZ = 1; // size = max - min
    std::vector<Color> pixels; // CPU buffer (RGB in 0..255)
};

extern BakedLightmap gBaked;   // define once in a .cpp
extern BakedLightmap gDynamic;   

void InitBakedLightmap128(int dungeonWidth, int dungeonHeight, float tileSize, float baseY);
void BakeStaticLightmapFromLights(const std::vector<LightSource>& lights);
void InitDynamicLightmapMatchBaked();
void BuildDynamicLightmapFromFrameLights(const std::vector<LightSample>& frameLights);