#pragma once

#include "raylib.h"
#include <string>
#include <vector>

struct WallInstance {
    Vector3 position;
    float rotationY;
    BoundingBox bounds;
};


void LoadDungeonLayout(const std::string& imagePath); // Just loads and caches image
void GenerateFloorTiles(float tileSize, float baseY);
void GenerateWallTiles(float tileSize, float baseY);
void GenerateCeilingTiles(float ceilingOffsetY);
void DrawDungeonFloor(Model floorTileModel);
void DrawDungeonWalls(Model wallSegmentModel);
void DrawDungeonCeiling(Model ceilingTileModel, float ceilingOffsetY);
