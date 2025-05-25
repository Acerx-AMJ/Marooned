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

std::vector<TreeInstance> GenerateTrees(Image heightmap, unsigned char* pixels, Vector3 terrainScale,
                                        float treeSpacing, float minTreeSpacing, float treeHeightThreshold);

std::vector<TreeInstance> FilterTreesAboveHeightThreshold(const std::vector<TreeInstance>& inputTrees, Image heightmap,
                                                          unsigned char* pixels, Vector3 terrainScale,
                                                          float treeHeightThreshold);
