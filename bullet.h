#pragma once

#include "raylib.h"
#include <vector>


class Bullet {
public:
    Bullet(Vector3 position, Vector3 direction, float speed, float lifetime);

    void Update(float deltaTime);
    void Draw() const;
    void kill();
    bool IsAlive() const;
    Vector3 GetPosition() const;
    

private:
    Vector3 position;
    Vector3 direction;
    float speed;
    bool alive;
    float age;
    float maxLifetime;
    float timer;
};


void FireBlunderbuss(Vector3 origin, Vector3 forward, float spreadDegrees, int pelletCount, float speed, float lifetime);
