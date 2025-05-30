#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "player.h"

enum class DinoState {
    Idle,
    Chase,
    Attack,
    RunAway
};


class Character {
public:
    Vector3 position;
    Texture2D* texture;
    int frameWidth, frameHeight;
    int currentFrame, maxFrames;
    int rowIndex;
    float animationTimer, animationSpeed;
    float scale;
    float rotationY = 0.0f; // in degrees
    DinoState state = DinoState::Idle;

    Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row = 0);
    void Update(float deltaTime, Vector3 playerPosition, Image heightmap, Vector3 terrainScale);
    void Draw(Camera3D camera);
    void SetAnimation(int row, int frames, float speed);
};

#endif
