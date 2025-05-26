// vegetation.cpp

#include "vegetation.h"
#include "raylib.h"
#include <vector>
#include <cmath>
#include "raymath.h"
#include "rlgl.h"

std::vector<TreeInstance> GenerateTrees(Image heightmap, unsigned char* pixels, Vector3 terrainScale,
                                        float treeSpacing, float minTreeSpacing, float treeHeightThreshold) {
    std::vector<TreeInstance> trees;

    for (int z = 0; z < heightmap.height; z += (int)treeSpacing) {
        for (int x = 0; x < heightmap.width; x += (int)treeSpacing) {
            int i = z * heightmap.width + x;
            float height = ((float)pixels[i] / 255.0f) * terrainScale.y;

            if (height > treeHeightThreshold) {
                Vector3 pos = {
                    (float)x / heightmap.width * terrainScale.x - terrainScale.x / 2,
                    height-5, //sink the tree into the ground a little. 
                    (float)z / heightmap.height * terrainScale.z - terrainScale.z / 2
                };

                // Check distance from other trees
                bool tooClose = false;
                for (const auto& other : trees) {
                    if (Vector3Distance(pos, other.position) < minTreeSpacing) {
                        tooClose = true;
                        break;
                    }
                }

                if (!tooClose) { //not too close, spawn a tree. 
                    TreeInstance tree;
                    tree.position = pos;
                    tree.rotationY = (float)GetRandomValue(0, 359);
                    tree.scale = 20.0f + ((float)GetRandomValue(0, 1000) / 100.0f); // 20.0 - 30.0
                    tree.yOffset = ((float)GetRandomValue(-200, 200)) / 100.0f;     // -2.0 to 2.0
                    tree.xOffset = ((float)GetRandomValue(-treeSpacing, treeSpacing));
                    tree.zOffset = ((float)GetRandomValue(-treeSpacing, treeSpacing));
                    tree.useAltModel = GetRandomValue(0, 1);


                    trees.push_back(tree);
                }
            }
        }
    }

    return trees;
}

std::vector<TreeInstance> FilterTreesAboveHeightThreshold(const std::vector<TreeInstance>& inputTrees, Image heightmap,
                                                          unsigned char* pixels, Vector3 terrainScale,
                                                          float treeHeightThreshold) {
    std::vector<TreeInstance> filtered;

    for (const auto& tree : inputTrees) {
        float xPercent = (tree.position.x + terrainScale.x / 2) / terrainScale.x;
        float zPercent = (tree.position.z + terrainScale.z / 2) / terrainScale.z;

        int xPixel = (int)(xPercent * heightmap.width);
        int zPixel = (int)(zPercent * heightmap.height);

        if (xPixel < 0 || xPixel >= heightmap.width || zPixel < 0 || zPixel >= heightmap.height) continue;

        int i = zPixel * heightmap.width + xPixel;
        float height = ((float)pixels[i] / 255.0f) * terrainScale.y;

        if (height > treeHeightThreshold) {
            filtered.push_back(tree);
        }
    }

    return filtered;
}

std::vector<BushInstance> GenerateBushes(Image heightmap, unsigned char* pixels, Vector3 terrainScale,
                                         float bushSpacing, float heightThreshold, Model bushModel) {
    std::vector<BushInstance> bushes;

    for (int z = 0; z < heightmap.height; z += (int)bushSpacing) {
        for (int x = 0; x < heightmap.width; x += (int)bushSpacing) {
            int i = z * heightmap.width + x;
            float height = ((float)pixels[i] / 255.0f) * terrainScale.y;

            if (height > heightThreshold) {
                Vector3 pos = {
                    (float)x / heightmap.width * terrainScale.x - terrainScale.x / 2,
                    height-5,
                    (float)z / heightmap.height * terrainScale.z - terrainScale.z / 2
                };

                BushInstance bush;
                bush.position = pos;
                bush.scale = 100.0f + ((float)GetRandomValue(0, 1000) / 100.0f);
                bush.model = bushModel;
                bush.yOffset = ((float)GetRandomValue(-200, 200)) / 100.0f;     // -2.0 to 2.0
                bush.xOffset = ((float)GetRandomValue(-bushSpacing*2, bushSpacing*2));
                bush.zOffset = ((float)GetRandomValue(-bushSpacing*2, bushSpacing*2));
                bushes.push_back(bush);
            }
        }
    }

    return bushes;
}

std::vector<BushInstance> FilterBushsAboveHeightThreshold(const std::vector<BushInstance>& inputTrees, Image heightmap,
                                                          unsigned char* pixels, Vector3 terrainScale,
                                                          float treeHeightThreshold) {
    std::vector<BushInstance> filtered;

    for (const auto& tree : inputTrees) {
        float xPercent = (tree.position.x + terrainScale.x / 2) / terrainScale.x;
        float zPercent = (tree.position.z + terrainScale.z / 2) / terrainScale.z;

        int xPixel = (int)(xPercent * heightmap.width);
        int zPixel = (int)(zPercent * heightmap.height);

        if (xPixel < 0 || xPixel >= heightmap.width || zPixel < 0 || zPixel >= heightmap.height) continue;

        int i = zPixel * heightmap.width + xPixel;
        float height = ((float)pixels[i] / 255.0f) * terrainScale.y;

        if (height > treeHeightThreshold) {
            filtered.push_back(tree);
        }
    }

    return filtered;
}

void DrawTrees(const std::vector<TreeInstance>& trees, Model& model1, Model& model2){
    for (const auto& tree : trees) {
        Vector3 pos = tree.position;
        pos.y += tree.yOffset;
        pos.x += tree.xOffset;
        pos.z += tree.zOffset;

        Model& treeModel = tree.useAltModel ? model1 : model2; // useAltModel is set when generating. 

        DrawModelEx(treeModel, pos, { 0, 1, 0 }, tree.rotationY,
                    { tree.scale, tree.scale, tree.scale }, WHITE);
    }

}

void DrawBushes(const std::vector<BushInstance>& bushes) {
    for (const auto& bush : bushes) {
        Vector3 pos = bush.position;
        pos.x += bush.xOffset;
        pos.y += bush.yOffset;
        pos.z += bush.zOffset;
        
        DrawModel(bush.model, pos, bush.scale, WHITE);
    }
}