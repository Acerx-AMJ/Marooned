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
Bullet::Bullet(Vector3 startPos, Vector3 vel, float lifetime, bool en, bool fb)
    : position(startPos),
      velocity(vel),
      alive(true),
      age(0.0f),
      maxLifetime(lifetime),
      enemy(en),
      fireball(fb)
{}


void Bullet::Update(Camera& camera, float deltaTime) {
    if (!alive) return;

    // Apply gravity
    if (!IsEnemy() && !isFireball()) velocity.y -= gravity * deltaTime; //enemy bullets don't have gravity
    
    // Move
    position = Vector3Add(position, Vector3Scale(velocity, deltaTime));

    // Age out
    age += deltaTime;
    if (age >= maxLifetime) alive = false;

    //hit floor or ceiling
    if (isDungeon){
        if (position.y <= dungeonPlayerHeight) kill(camera);
        if (position.y >= ceilingHeight) kill(camera);     
    }else{
        if (position.y <= waterHeightY) kill(camera);       
    }

}


void Bullet::Draw() const {
    if (!alive) return;
    if (fireball){
        DrawSphere(position, 20, YELLOW);

    }else{
        DrawSphere(position, 1.5f, WHITE); // simple debug visualization
    }
    

}

bool Bullet::IsAlive() const {
    return alive;
}

bool Bullet::IsEnemy() const {
    return enemy;
}

bool Bullet::isFireball() const {
    return fireball;
}

void Bullet::kill(Camera& camera){
    //smoke decals and bullet death
    Vector3 camDir = Vector3Normalize(Vector3Subtract(position, camera.position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, -100.0f));

    decals.emplace_back(offsetPos, DecalType::Smoke, &smokeSheet, 7, 0.8f, 0.1f, 25.0f);

    alive = false;
    
}

void Bullet::Blood(Camera camera){
    //spawn blood decals at bullet position, if it's not a skeleton.
    Vector3 camDir = Vector3Normalize(Vector3Subtract(position, camera.position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, -100.0f));

    decals.emplace_back(offsetPos, DecalType::Blood, &bloodSheet, 7, 1.0f, 0.1f, 60.0f);

    alive = false;

}

Vector3 Bullet::GetPosition() const {
    return position;
}



void FireBlunderbuss(Vector3 origin, Vector3 forward, float spreadDegrees, int pelletCount, float speed, float lifetime, bool enemy) {
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
        activeBullets.emplace_back(origin, velocity, lifetime, enemy);

    }
}

void FireBullet(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy) {
    Vector3 direction = Vector3Subtract(target, origin);
    direction = Vector3Normalize(direction);
    Vector3 velocity = Vector3Scale(direction, speed);

    activeBullets.emplace_back(origin, velocity, lifetime, enemy);
}

void FireFireball(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy, bool fireball){
    Vector3 direction = Vector3Subtract(target, origin);
    direction = Vector3Normalize(direction);
    Vector3 velocity = Vector3Scale(direction, speed);

    activeBullets.emplace_back(origin, velocity, lifetime, enemy, fireball);

}
