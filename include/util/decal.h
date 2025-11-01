#pragma once
#include "raylib.h"
#include "raymath.h"

// What kind of decal this is
enum class DecalType {
    Smoke,
    Blood,
    Explosion,
    MagicAttack,
    NewBlood,
    Blocked,
    MeleeSwipe,
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
    Vector3 velocity;
    float drag = 1.0f;

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

        if (type == DecalType::MeleeSwipe){
                    // Kinematics
            position = Vector3Add(position, Vector3Scale(velocity, deltaTime));
            // Exponential drag (stable, framerate-independent)
            float damp = expf(-drag * deltaTime);
            velocity = Vector3Scale(velocity, damp);

        }


        currentFrame = static_cast<int>(timer / frameTime);
        if (currentFrame >= maxFrames) currentFrame = maxFrames - 1;
    }
    //decals are drawn in transparentDraw after GatherDecals


};