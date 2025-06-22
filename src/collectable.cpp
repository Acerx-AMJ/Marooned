#include "collectable.h"
#include "raymath.h"
#include <iostream>

Collectable::Collectable(CollectableType type, Vector3 position)
    : type(type), position(position)
{
    collider = {
        position - Vector3{20, 0, 20},
        position + Vector3{20, 60, 20}
    };
}

void Collectable::Update(float deltaTime) {
    bobTimer += deltaTime;
    float bobAmount = sinf(bobTimer)* 0.1f;
    position.y += bobAmount;

    // Update collider height to match
    collider.min.y = position.y;
    collider.max.y = position.y + 60.0f;
}

void Collectable::Draw(Texture2D icon, const Camera& camera, float scale) const {
    DrawBillboard(camera, icon, position, scale, WHITE);
   
}

bool Collectable::CheckPickup(const Vector3& playerPos, float pickupRadius) const {
    return Vector3Distance(playerPos, position) < pickupRadius;
}

