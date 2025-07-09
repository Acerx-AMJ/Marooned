#pragma once
#include "raylib.h"
#include <vector>
#include "decal.h"
#include "weapon.h"

enum BillboardType {
    Billboard_FacingCamera,
    Billboard_FixedFlat,
    Billboard_Decal,
    Billboard_MuzzleFlash
};

struct BillboardDrawRequest {
    BillboardType type;
    Vector3 position;
    Texture2D* texture;
    Rectangle sourceRect;
    float size;
    Color tint;
    float distanceToCamera;
    float rotationY;
};

extern std::vector<BillboardDrawRequest> billboardRequests;

// Expose a public function to gather everything
void GatherTransparentDrawRequests(Camera& camera, Weapon& weapon, float deltaTime);

// Expose a public function to draw them all
void DrawTransparentDrawRequests(Camera& camera);
void GatherEnemies(Camera& camera);
void GatherDungeonFires(Camera& camera, float deltaTime);
void GatherWebs(Camera& camera);
void GatherDecals(Camera& camera, const std::vector<Decal>& decals);