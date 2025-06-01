#include "bullet.h"
#include "raymath.h"
#include <raylib.h>
#include <cmath>
#include "world.h"

Bullet::Bullet(Vector3 startPos, Vector3 dir, float spd, float lifetime)
    : position(startPos), direction(Vector3Normalize(dir)), speed(spd), alive(true), age(0.0f), maxLifetime(lifetime) {}

void Bullet::Update(float deltaTime) {
    if (!alive) return;

    // Move the bullet
    position = Vector3Add(position, Vector3Scale(direction, speed * deltaTime));

    // Age the bullet
    age += deltaTime;
    if (age >= maxLifetime) alive = false;
}

void Bullet::Draw() const {
    if (!alive) return;

    DrawSphere(position, 1.5f, WHITE); // simple debug visualization
}

bool Bullet::IsAlive() const {
    return alive;
}

void Bullet::kill(){
    alive = false;
    
}

Vector3 Bullet::GetPosition() const {
    return position;
}

#include "bullet.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>


void FireBlunderbuss(Vector3 origin, Vector3 forward, float spreadDegrees, int pelletCount, float speed, float lifetime) {
    for (int i = 0; i < pelletCount; ++i) {
        // Convert spread from degrees to radians
        float spreadRad = spreadDegrees * DEG2RAD;

        // Random horizontal and vertical spread
        float yaw = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * spreadRad;
        float pitch = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * spreadRad;

        // Create rotation matrix from yaw and pitch
        Matrix rotYaw = MatrixRotateY(yaw);
        Matrix rotPitch = MatrixRotate(Vector3CrossProduct(forward, { 0, 1, 0 }), pitch);
        Matrix spreadMatrix = MatrixMultiply(rotYaw, rotPitch);

        Vector3 dir = Vector3Transform(forward, spreadMatrix);
        dir = Vector3Normalize(dir);

        activeBullets.emplace_back(origin, dir, speed, lifetime);
    }
}

