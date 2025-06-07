#pragma once

#include "raylib.h"
#include <string>
#include <vector>

struct WallInstance {
    Vector3 position;
    float rotationY;
    BoundingBox bounds;
};

struct WallRun {
    Vector3 startPos;
    Vector3 endPos;
    float rotationY;
    BoundingBox bounds; // Precomputed, for fast collision checking
};

extern std::vector<WallRun> wallRunColliders;

void LoadDungeonLayout(const std::string& imagePath); // Just loads and caches image
void GenerateFloorTiles(float tileSize, float baseY);
void GenerateWallTiles(float tileSize, float baseY);
void GenerateCeilingTiles(float ceilingOffsetY);
void DrawDungeonFloor(Model floorTileModel);
void DrawDungeonWalls(Model wallSegmentModel);
void DrawDungeonCeiling(Model ceilingTileModel, float ceilingOffsetY);
void ResolveBoxSphereCollision(const BoundingBox& box, Vector3& position, float radius);
void ClearDungeon();
