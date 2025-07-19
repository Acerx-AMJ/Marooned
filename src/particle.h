#pragma once
#include "raylib.h"
#include "raymath.h"
#include <iostream>
struct Particle {
    Vector3 position;
    Vector3 velocity;
    float life;         // Remaining life
    float maxLife;      // Total life span
    Color color;
    float size;
    bool active = false;

    void Update(float dt) {
        if (!active) return;

        velocity.y -= 980.0f * dt; 
        position = Vector3Add(position, Vector3Scale(velocity, dt));
        life -= dt;


        // Optional fade out
        float t = life / maxLife;
        color.a = (unsigned char)(255 * t);

        if (life <= 0.0f) active = false;
    }

    void Draw(Camera3D camera) const {
        if (!active) return;
        std::cout << "drawing particles\n";
        // Optional: make billboard face the camera
        Vector2 screenPos = GetWorldToScreen(position, camera);
        DrawSphere(position, 2, WHITE);

    }
};
