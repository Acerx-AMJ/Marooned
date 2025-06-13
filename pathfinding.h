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
bool HasLineOfSightBFS(int startX, int startY, int goalX, int goalY, const std::vector<std::vector<bool>>& walkable);
std::vector<Vector2> FindPath(Vector2 start, Vector2 goal);