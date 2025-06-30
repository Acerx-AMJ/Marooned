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

bool HasWorldLineOfSight(Vector3 from, Vector3 to);
bool LineOfSightRaycast(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps);
bool SingleRayBlocked(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps);
Vector2 GetRandomReachableTile(const Vector2& start, const Character* self, int maxAttempts = 100);
bool TrySetRandomPatrolPath(const Vector2& start, Character* self, std::vector<Vector3>& outPath);
std::vector<Vector2> SmoothPath(const std::vector<Vector2>& path, const Image& dungeonMap);
std::vector<Vector2> FindPath(Vector2 start, Vector2 goal);