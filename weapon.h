#pragma once

#include "raylib.h"

struct Weapon {
    Model model;
    Vector3 scale = { 1.0f, 1.0f, 1.0f };

    float forwardOffset = 80.0f;
    float sideOffset = 20.0f;
    float verticalOffset = -30.0f;

    float recoil = 0.0f;
    float recoilAmount = 15.0f;
    float recoilRecoverySpeed = 30.0f;

    Sound fireSound;
    float lastFired = -999.0f;
    float fireCooldown = 1.0f;

    Texture2D muzzleFlashTexture;
    float flashDuration = 0.1f;
    float flashTimer = 0.0f;

    void Fire(Camera& camera);
    void Update(float deltaTime);
    void Draw(const Camera& camera);
};

