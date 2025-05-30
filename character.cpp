#include "Character.h"
#include "raymath.h"
#include <iostream>


Character::Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row)
    : position(pos), texture(tex), frameWidth(fw), frameHeight(fh),
      currentFrame(0), maxFrames(frames), animationTimer(0), animationSpeed(speed),
      scale(scl), rowIndex(row) {}

void Character::Update(float deltaTime, Vector3 playerPosition, Image heightmap, Vector3 terrainScale) {
    animationTimer += deltaTime;

    float groundY = GetHeightAtWorldPosition(position, heightmap, terrainScale);

    // Gravity
    float gravity = 400.0f; // scale this to fit your world
    static float verticalVelocity = 0.0f;

    float spriteHeight = frameHeight * scale;

    if (position.y > groundY + spriteHeight / 2.0f) {
        verticalVelocity -= gravity * deltaTime;
        position.y += verticalVelocity * deltaTime;
    } else {
        verticalVelocity = 0.0f;
        position.y = groundY + spriteHeight / 2.0f;
    }
    float distance = Vector3Distance(position, playerPosition);

    // Adjust distances for your scale (example values)
    switch (state) {
        case DinoState::Idle:
            if (distance < 3000.0f) {
                state = DinoState::Chase;
                SetAnimation(1, 4, 0.12f);
            }
            break;

        case DinoState::Chase:
            if (distance < 100.0f) {
                state = DinoState::Attack;
                SetAnimation(2, 5, 0.1f);
            } else if (distance > 10000.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
            } else {
                Vector3 dir = Vector3Normalize(Vector3Subtract(playerPosition, position));
                Vector3 horizontalMove = { dir.x, 0, dir.z };
                position = Vector3Add(position, Vector3Scale(horizontalMove, deltaTime * 300.0f)); // adjust speed
                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
            }
            break;

        case DinoState::Attack:
            if (distance > 200.0f) {
                state = DinoState::Chase;
                SetAnimation(1, 4, 0.12f);
            }
            break;

        case DinoState::RunAway:
            // TODO: Add logic for fleeing
            break;
    }

    if (animationTimer >= animationSpeed) {
        animationTimer = 0;
        currentFrame = (currentFrame + 1) % maxFrames;
    }
}



void Character::Draw(Camera3D camera) {
    Rectangle sourceRec = {
        (float)(currentFrame * frameWidth),
        (float)(rowIndex * frameHeight),
        (float)frameWidth,
        (float)frameHeight
    };

    Vector2 size = { frameWidth * scale, frameHeight * scale };

    DrawBillboardRec(camera, *texture, sourceRec, position, size, WHITE);
}

void Character::SetAnimation(int row, int frames, float speed) {
    rowIndex = row;
    maxFrames = frames;
    animationSpeed = speed;
    currentFrame = 0;
    animationTimer = 0;
}
