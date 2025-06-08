#pragma once

#include "raylib.h"
#include <string>
#include <vector>

struct WallInstance {
    Vector3 position;
    float rotationY;
    BoundingBox bounds;
    Color tint = WHITE;  // Default to no tinting
};

struct WallRun {
    Vector3 startPos;
    Vector3 endPos;
    float rotationY;
    BoundingBox bounds; // Precomputed, for fast collision checking
};

struct FloorTile {
    Vector3 position;
    Color tint;
};

struct CeilingTile {
    Vector3 position;
    Color tint;
};


extern std::vector<WallRun> wallRunColliders;

void LoadDungeonLayout(const std::string& imagePath); // Just loads and caches image
void GenerateFloorTiles(float tileSize, float baseY);
void GenerateWallTiles(float tileSize, float baseY);
void GenerateCeilingTiles(float ceilingOffsetY);
void DrawDungeonFloor(Model floorTileModel);
void DrawDungeonWalls(Model wallSegmentModel);
//void DrawDungeonCeiling(Model ceilingTileModel, float ceilingOffsetY);
void DrawDungeonCeiling(Model ceilingTileModel);
void ResolveBoxSphereCollision(const BoundingBox& box, Vector3& position, float radius);
void UpdateWallTints(Vector3 playerPos);
void UpdateFloorTints(Vector3 playerPos);
void UpdateCeilingTints(Vector3 playerPos);
void ClearDungeon();
