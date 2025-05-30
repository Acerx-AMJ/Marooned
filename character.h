#pragma once
#include "raylib.h"

struct Character {
    Vector3 position;
    Texture2D* texture;
    int frameWidth;
    int frameHeight;
    int currentFrame;
    int maxFrames;
    float animationTimer;
    float animationSpeed;
    float scale;

    Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl = 1.0f);

    void Update(float deltaTime);
    void Draw(Camera3D camera);
};
