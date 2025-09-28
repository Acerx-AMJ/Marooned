// vegetation.cpp

#include "vegetation.h"
#include "raylib.h"
#include <vector>
#include <cmath>
#include "raymath.h"
#include "rlgl.h"
#include "resourceManager.h"
#include "world.h"
#include "algorithm"
#include "shadows.h"

std::vector<TreeInstance> trees;
std::vector<BushInstance> bushes;
std::vector<const TreeInstance*> sortedTrees;




void sortTrees(Camera& camera){
    // Sort farthest to nearest (back-to-front)
    std::sort(sortedTrees.begin(), sortedTrees.end(), [&](const TreeInstance* a, const TreeInstance* b) {
        float da = Vector3Distance(camera.position, a->position);
        float db = Vector3Distance(camera.position, b->position);
        return da > db; // draw farther trees first
    });
}



void generateVegetation(){

    float treeSpacing = 150.0f;
    float minTreeSpacing = 50.0f;
    float treeHeightThreshold = terrainScale.y * 0.8f;
    float bushHeightThreshold = terrainScale.y * 0.9f;
    heightmapPixels = (unsigned char*)heightmap.data; //for iterating heightmap data for tree placement
    // 🌴 Generate the trees
    trees = GenerateTrees(heightmap, heightmapPixels, terrainScale, treeSpacing, minTreeSpacing, treeHeightThreshold);



    // 🌴 Filter trees based on final height cutoff
    trees = FilterTreesAboveHeightThreshold(trees, heightmap, heightmapPixels, terrainScale, treeHeightThreshold);

    bushes = GenerateBushes(heightmap, heightmapPixels, terrainScale, treeSpacing, bushHeightThreshold);
    bushes = FilterBushsAboveHeightThreshold(bushes, heightmap, heightmapPixels, terrainScale, bushHeightThreshold);

    // Copy tree pointers into a separate list so we can sort them by distance
    // without modifying the original `trees` vector (which holds the actual data)
    for (const auto& tree : trees) { //solves tree leaf glitches. 
        sortedTrees.push_back(&tree);
    }

    // Define world XZ bounds from your terrainScale (centered at origin)
    Rectangle worldXZ = {
        -terrainScale.x * 0.5f,   // minX
        -terrainScale.z * 0.5f,   // minZ
        terrainScale.x,          // sizeX
        terrainScale.z           // sizeZ
    };

    // Create/update a global or stored mask
    //static TreeShadowMask gTreeShadowMask;
    InitOrResizeTreeShadowMask(gTreeShadowMask, /*tex size*/ 1024, 1024, worldXZ);

    // Bake
    BuildTreeShadowMask(gTreeShadowMask, trees,
        /*baseRadiusMeters*/ 4.5f,  /*darknessCenter*/ 0.55f, /*rings*/ 10);

    
}

std::vector<TreeInstance> GenerateTrees(Image& heightmap, unsigned char* pixels, Vector3 terrainScale,
                                        float treeSpacing, float minTreeSpacing, float treeHeightThreshold) {
    std::vector<TreeInstance> trees;
    Vector3 ePos = dungeonEntrances[0].position; 
    float startClearRadiusSq = 700.0f;
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

                    if (Vector3Distance(pos, startPosition) < startClearRadiusSq) tooClose = true;

                    for (DungeonEntrance& d : dungeonEntrances){//dont spawn trees ontop of entrances
                        
                        if (Vector3Distance(pos, d.position) < treeSpacing * 2 ){
                            
                            tooClose = true;
                            break;
                        }
                    }
                }

                if (!tooClose) { //not too close, spawn a tree. 
                    TreeInstance tree;
                    tree.position = pos;
                    tree.rotationY = (float)GetRandomValue(0, 359);
                    tree.scale = 20.0f + ((float)GetRandomValue(0, 1000) / 100.0f); // 20.0 - 30.0
                    tree.yOffset = ((float)GetRandomValue(-600, 200)) / 100.0f;     // -2.0 to 2.0
                    tree.xOffset = ((float)GetRandomValue(-treeSpacing, treeSpacing));
                    tree.zOffset = ((float)GetRandomValue(-treeSpacing, treeSpacing));
                    tree.useAltModel = GetRandomValue(0, 1);
                    tree.cullFactor = 1.05f; //5 percent higher than tree threshold. 


                    trees.push_back(tree);
                }
            }
        }
    }

    return trees;
}

std::vector<TreeInstance> FilterTreesAboveHeightThreshold(const std::vector<TreeInstance>& inputTrees, Image& heightmap,
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

        if (height > treeHeightThreshold * tree.cullFactor) {
            filtered.push_back(tree);
        }
    }

    return filtered;
}

std::vector<BushInstance> GenerateBushes(Image& heightmap, unsigned char* pixels, Vector3 terrainScale,
                                         float bushSpacing, float heightThreshold) {
    std::vector<BushInstance> bushes;
    float startClearRadiusSq = 700.0f;  
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

                if (Vector3Distance(pos, startPosition) < startClearRadiusSq) continue;
            
                BushInstance bush;
                bush.position = pos;
                bush.scale = 100.0f + ((float)GetRandomValue(0, 1000) / 100.0f);
                bush.model = R.GetModel("bush");
                bush.yOffset = ((float)GetRandomValue(-200, 200)) / 100.0f;     // -2.0 to 2.0
                bush.xOffset = ((float)GetRandomValue(-bushSpacing*2, bushSpacing*2));
                bush.zOffset = ((float)GetRandomValue(-bushSpacing*2, bushSpacing*2)); //space them out wider, then cull more aggresively. 
                bush.cullFactor = 1.09f; //agressively cull bushes. 
                bushes.push_back(bush);
            }
        }
    }

    return bushes;
}

std::vector<BushInstance> FilterBushsAboveHeightThreshold(const std::vector<BushInstance>& inputBushes, Image& heightmap,
                                                          unsigned char* pixels, Vector3 terrainScale,
                                                          float treeHeightThreshold) {
    std::vector<BushInstance> filtered;

    for (const auto& bush : inputBushes) {
        float xPercent = (bush.position.x + terrainScale.x / 2) / terrainScale.x;
        float zPercent = (bush.position.z + terrainScale.z / 2) / terrainScale.z;

        int xPixel = (int)(xPercent * heightmap.width);
        int zPixel = (int)(zPercent * heightmap.height);

        if (xPixel < 0 || xPixel >= heightmap.width || zPixel < 0 || zPixel >= heightmap.height) continue;

        int i = zPixel * heightmap.width + xPixel;
        float height = ((float)pixels[i] / 255.0f) * terrainScale.y;

        if (height > treeHeightThreshold * bush.cullFactor) {
            filtered.push_back(bush);
        }
    }

    return filtered;
}

void RemoveAllVegetation() {
    trees.clear();
    bushes.clear();
    sortedTrees.clear();
}



void DrawTrees(const std::vector<TreeInstance>& trees, Model& shadowQuad, Camera& camera){
    //sortTrees(camera); //sort trees by distance to camera
    for (const TreeInstance* tree : sortedTrees) {
        Vector3 pos = tree->position;
        pos.y += tree->yOffset;
        pos.x += tree->xOffset;
        pos.z += tree->zOffset;

        Model& treeModel = tree->useAltModel ? R.GetModel("palmTree") : R.GetModel("palm2");

        DrawModelEx(treeModel, pos, { 0, 1, 0 }, tree->rotationY,
                    { tree->scale, tree->scale, tree->scale }, WHITE);

        // Vector3 shadowPos = {
        //     tree->position.x + tree->xOffset,
        //     tree->position.y + 9.0f,
        //     tree->position.z + tree->zOffset
        // };

        // Shader shadowSh = R.GetShader("shadowShader"); // your quad shadow shader
        // int locStrength = GetShaderLocation(shadowSh, "shadowStrength");

        // // Draw trees (lighter)
        // float treeStrength = 0.25f;
        // SetShaderValue(shadowSh, locStrength, &treeStrength, SHADER_UNIFORM_FLOAT);


        // DrawModelEx(shadowQuad, shadowPos, {0, 1, 0}, 0,
        //             {tree->scale * 15.0f, 1.0f, tree->scale * 15.0f}, WHITE);
    }


}

void DrawBushes(const std::vector<BushInstance>& bushes, Model& shadowQuad) {
    for (const auto& bush : bushes) {
        Vector3 pos = bush.position;
        pos.x += bush.xOffset;
        pos.y += bush.yOffset-10;
        pos.z += bush.zOffset;
        Color customGreen = { 0, 160, 0, 255 };
        DrawModel(bush.model, pos, bush.scale, customGreen);

            //draw shadow decal under bushes. 
        // Vector3 shadowPos = {
        //     bush.position.x + bush.xOffset,
        //     bush.position.y + bush.yOffset + 10.0f, // Slightly above ground to prevent Z-fighting
        //     bush.position.z + bush.zOffset
        // };

        
        // DrawModelEx(
        //     shadowQuad,
        //     shadowPos,
        //     {0, 1, 0},
        //     0,
        //     {bush.scale * 5.0f, 1.0f, bush.scale * 5.0f}, // XZ scale, flat on Y
        //     WHITE
        // );
    }
}