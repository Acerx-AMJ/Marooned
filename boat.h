#ifndef BOAT_H
#define BOAT_H

#include "raylib.h"

struct Boat {
    Vector3 position;
    Vector3 velocity;
    float rotationY;  // heading
    float speed;
    float maxSpeed;
    float acceleration;
    float turnSpeed;

    bool playerOnBoard;
};

extern Boat player_boat;

void InitBoat(Boat& boat, Vector3 startPos);
void UpdateBoat(Boat& boat, float deltaTime);
void DrawBoat(const Boat& boat);

#endif
