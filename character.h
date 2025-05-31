#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "player.h"
#include <vector>

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
    Vector3 previousPosition;
    DinoState state = DinoState::Idle;
    int frameWidth, frameHeight;
    int currentFrame, maxFrames;
    int rowIndex;
    float animationTimer, animationSpeed;
    float scale;
    float rotationY = 0.0f; // in degrees
    float stateTimer = 0.0f;
    float attackTimer = 0.0f;


    Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row = 0);
    //void Update(float deltaTime, Vector3 playerPosition, Image heightmap, Vector3 terrainScale);
    void Update(float deltaTime, Vector3 playerPosition, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors);
    Vector3 ComputeRepulsionForce(const std::vector<Character*>& allRaptors, float repulsionRadius = 500.0f, float repulsionStrength = 6000.0f);

    void Draw(Camera3D camera);
    void SetAnimation(int row, int frames, float speed);
};

#endif
