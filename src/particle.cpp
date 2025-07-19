#include "Particle.h"
#include "raymath.h"
#include <iostream>

void Particle::Update(float dt) {
    if (!active) return;

    velocity.y -= gavity * dt;
    position = Vector3Add(position, Vector3Scale(velocity, dt));
    life -= dt;

    float t = life / maxLife;
    color.a = (unsigned char)(255 * t);

    if (life <= 0.0f) active = false;
}

void Particle::Draw(Camera3D camera) const {
    if (!active) return;

    Vector2 screenPos = GetWorldToScreen(position, camera);
    DrawSphere(position, 5.0f, color);
}
