#pragma once

#include "raylib.h"
#include <string>
#include <vector>

struct BarrelInstance {
    Vector3 position;
    Color tint = WHITE;
};

struct PillarInstance {
    Vector3 position;
    float rotation;
};

struct LightSource {
    Vector3 position;
    float intensity = 0.5;  // maybe 1.0 = full bright, 0.5 = dim, etc.
    float range = 1500;
    float lifeTime = 1.0f;
    float age;

};



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

extern std::vector<PillarInstance> pillars;
extern std::vector<LightSource> dungeonLights;
extern std::vector<LightSource> bulletLights;
extern std::vector<WallRun> wallRunColliders;
extern std::vector<BarrelInstance> barrelInstances;
extern Image dungeonImg;
extern Color* dungeonPixels;
extern int dungeonWidth;
extern int dungeonHeight;


void LoadDungeonLayout(const std::string& imagePath); // Just loads and caches image
void GenerateFloorTiles(float tileSize, float baseY);
void GenerateWallTiles(float tileSize, float baseY);
void GenerateCeilingTiles(float ceilingOffsetY);
void GenerateBarrels(float tileSize, float baseY);
void GenerateLightSources(float tileSize, float baseY);
void DrawDungeonFloor(Model floorTileModel);
void DrawDungeonWalls(Model wallSegmentModel);
void DrawDungeonFloor(Model floorModel);
void DrawDungeonBarrels(Model barrelModel);
void DrawDungeonPillars(Model pillarModel);
//void DrawDungeonCeiling(Model ceilingTileModel, float ceilingOffsetY);
void DrawDungeonCeiling(Model ceilingTileModel);
void ResolveBoxSphereCollision(const BoundingBox& box, Vector3& position, float radius);
void UpdateWallTints(Vector3 playerPos);
void UpdateFloorTints(Vector3 playerPos);
void UpdateCeilingTints(Vector3 playerPos);
void UpdateBarrelTints(Vector3 playerPos);
bool IsDungeonFloorTile(int x, int y); 
Vector3 GetDungeonWorldPos(int x, int y, float tileSize, float baseY);
Vector3 FindSpawnPoint(Color* pixels, int width, int height, float tileSize, float baseY);
void ClearDungeon();
