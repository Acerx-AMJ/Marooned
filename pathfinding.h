#pragma once
#include "raylib.h"
#include <vector>

extern std::vector<std::vector<bool>> walkable;
class Character;  // ✅ This is the key line
void ConvertImageToWalkableGrid(const Image& dungeonMap);
Vector2 WorldToImageCoords(Vector3 worldPos);
bool IsWalkable(int x, int y);
bool IsTileOccupied(int x, int y, const std::vector<Character*>& skeletons, const Character* self);
Character* GetTileOccupier(int x, int y, const std::vector<Character*>& skeletons, const Character* self);
bool LineOfSightRaycast(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps);
std::vector<Vector2> SmoothPath(const std::vector<Vector2>& path, const Image& dungeonMap);
std::vector<Vector2> FindPath(Vector2 start, Vector2 goal);