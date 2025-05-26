// vegetation.h

#pragma once

#include "raylib.h"
#include <vector>

struct TreeInstance {
    Vector3 position;
    float rotationY;
    float scale;
    float yOffset;
    float xOffset;
    float zOffset;
};

struct BushInstance {
    Vector3 position;
    Model model;
    float scale;
    float rotationY;
    float yOffset;
    float xOffset;
    float zOffset;
};

std::vector<TreeInstance> GenerateTrees(Image heightmap, unsigned char* pixels, Vector3 terrainScale,
                                        float treeSpacing, float minTreeSpacing, float treeHeightThreshold);

std::vector<TreeInstance> FilterTreesAboveHeightThreshold(const std::vector<TreeInstance>& inputTrees, Image heightmap,
                                                          unsigned char* pixels, Vector3 terrainScale,
                                                          float treeHeightThreshold);

void DrawBushes(const std::vector<BushInstance>& bushes);
std::vector<BushInstance> GenerateBillboardBushes(Image heightmap, Vector3 terrainScale, float spacing, float minHeight, float maxHeight);
std::vector<BushInstance> GenerateBushes(Image heightmap, unsigned char* pixels, Vector3 terrainScale,
                                         float bushSpacing, float heightThreshold, Model bushModel);