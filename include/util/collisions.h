#pragma once
#include "raylib.h"
#include "char/player.h"
#include "char/character.h"
#include "world/vegetation.h"

void UpdateCollisions(Camera& camera);
void CheckBulletHits(Camera& camera);
void HandleMeleeHitboxCollision(Camera& camera);
bool CheckCollisionPointBox(Vector3 point, BoundingBox box);
void HandleDoorInteraction(Camera& camera);
void DoorCollision();
void TreeCollision(Camera& camera);
bool CheckTreeCollision(const TreeInstance& tree, const Vector3& playerPos);
void ResolveTreeCollision(const TreeInstance& tree, Vector3& playerPos);
void HandleEnemyPlayerCollision(Player* player);
bool CheckBulletHitsTree(const TreeInstance& tree, const Vector3& bulletPos);
void ResolveBoxSphereCollision(const BoundingBox& box, Vector3& position, float radius);
void ResolvePlayerEnemyMutualCollision(Character* enemy, Player* player);
void WallCollision();
void barrelCollision();
void SpiderWebCollision();
void ChestCollision();
void pillarCollision(); 
void launcherCollision();