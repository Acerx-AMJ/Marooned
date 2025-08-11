#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>

// What kind of decal this is
enum class DecalType {
    Smoke,
    Blood,
    Explosion,
    // Add more types later
};

struct Decal {
    Vector3 position;
    DecalType type;
    Texture2D texture;
    int maxFrames = 1;
    float lifetime = 1.0f;
    float frameTime = 0.1f;
    float size = 1.0f;
    float timer = 0.0f;
    int currentFrame = 0;
    bool alive = true;
    
    Decal(Vector3 pos, DecalType t, Texture2D tex, int frameCount, float life, float frameDuration, float scale = 1.0f)
        : position(pos), type(t), texture(tex), maxFrames(frameCount),
          lifetime(life), frameTime(frameDuration), size(scale)
    {}

    void Update(float deltaTime) {
        timer += deltaTime;

        if (timer >= lifetime) {
            alive = false;
            return;
        }

        currentFrame = static_cast<int>(timer / frameTime);
        if (currentFrame >= maxFrames) currentFrame = maxFrames - 1;
    }

void Draw() {
    if (!alive) return;

    Rectangle sourceRec;

    if (type == DecalType::Explosion) {
        sourceRec = {
            static_cast<float>(currentFrame * 196),
            0,
            196,
            190
        };
    } else {
        sourceRec = {
            static_cast<float>(currentFrame * 64),
            0,
            64,
            64
        };
    }

    //Vector2 drawSize = { size, size };
    //DrawBillboardRec(camera, *texture, sourceRec, position, drawSize, WHITE);
}


};