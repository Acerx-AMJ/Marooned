#include "bullet.h"
#include "raymath.h"
#include <raylib.h>
#include <cmath>
#include "world.h"
#include "resources.h"
#include "decal.h"

// Bullet::Bullet(Vector3 startPos, Vector3 dir, float spd, float lifetime)
//     : position(startPos), direction(Vector3Normalize(dir)), speed(spd), alive(true), age(0.0f), maxLifetime(lifetime) {}

// New constructor matching velocity-based logic
Bullet::Bullet(Vector3 startPos, Vector3 vel, float lifetime)
    : position(startPos),
      velocity(vel),
      alive(true),
      age(0.0f),
      maxLifetime(lifetime)
{}


void Bullet::Update(Camera& camera, float deltaTime) {
    if (!alive) return;

    // Apply gravity
    velocity.y -= gravity * deltaTime;

    // Move
    position = Vector3Add(position, Vector3Scale(velocity, deltaTime));

    // Age out
    age += deltaTime;
    if (age >= maxLifetime) alive = false;

    //hit floor
    if (isDungeon){
        if (position.y <= dungeonPlayerHeight) kill(camera);     
    }else{
        if (position.y <= player.groundY) kill(camera);       
    }

}


void Bullet::Draw() const {
    if (!alive) return;

    DrawSphere(position, 1.5f, WHITE); // simple debug visualization
}

bool Bullet::IsAlive() const {
    return alive;
}

void Bullet::kill(Camera& camera){
    //smoke decals and bullet death
    Vector3 camDir = Vector3Normalize(Vector3Subtract(position, camera.position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, -100.0f));

    decals.emplace_back(offsetPos, DecalType::Smoke, &smokeSheet, 7, 0.8f, 0.1f, 25.0f);

    alive = false;
    
}

Vector3 Bullet::GetPosition() const {
    return position;
}



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

        Vector3 velocity = Vector3Scale(dir, speed);
        activeBullets.emplace_back(origin, velocity, lifetime);

    }
}

