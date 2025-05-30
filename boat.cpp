#include "boat.h"
#include "raymath.h"
#include "resources.h"
#include "world.h"

Boat player_boat{};

void InitBoat(Boat& boat, Vector3 startPos) {
    boat.position = startPos;
    boat.velocity = { 0.0f, 0.0f, 0.0f };
    boat.rotationY = 0.0f;
    boat.speed = 0.0f;
    boat.maxSpeed = 400.0f;
    boat.acceleration = 200.0f;
    boat.turnSpeed = 20.0f; // degrees per second
    boat.playerOnBoard = false;
}

void UpdateBoat(Boat& boat, float deltaTime) {
    if (!boat.playerOnBoard) return;

    // Turning
    if (controlPlayer){
        if (IsKeyDown(KEY_D))  boat.rotationY -= boat.turnSpeed * deltaTime;
        if (IsKeyDown(KEY_A)) boat.rotationY += boat.turnSpeed * deltaTime;

        // Forward/slowdown
        if (IsKeyDown(KEY_W)) {
            boat.speed += boat.acceleration * deltaTime;
        }else if (IsKeyDown(KEY_S)){
            boat.speed -= boat.acceleration * deltaTime;
        } else {
            boat.speed *= 0.98f; // drag
        }

    }




    boat.speed = Clamp(boat.speed, -boat.maxSpeed, boat.maxSpeed);

    // Movement
    float radians = DEG2RAD * boat.rotationY;
    Vector3 direction = { sinf(radians), 0.0f, cosf(radians) };
    boat.velocity = Vector3Scale(direction, boat.speed);
    boat.position = Vector3Add(boat.position, Vector3Scale(boat.velocity, deltaTime));
}

void DrawBoat(const Boat& boat) {
    float bob = sinf(GetTime() * 2.0f) * 0.1f;
    Vector3 drawPos = boat.position;
    drawPos.y += bob;

    DrawModelEx(boatModel, drawPos, {0, 1, 0}, boat.rotationY, {1.0f, 1.0f, 1.0f}, WHITE);
}
