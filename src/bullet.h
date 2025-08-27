#pragma once

#include "raylib.h"
#include <vector>
#include "emitter.h"

enum class BulletType {
    Default,
    Fireball,
    Iceball,
   
};

struct BulletLight {
    bool   active = false;        // has light?
    Vector3 color = {1,1,1};      // 0..1
    float  range = 0.f;           // > 0
    float  intensity = 0.f;       // >= 0

    // Optional post-explosion glow
    bool   detachOnDeath = true;
    bool   detached = false;      // became a glow after bullet died
    float  age = 0.f;
    float  lifeTime = 0.f;        // e.g. 0.25s
    Vector3 posWhenDetached{};    // freeze final position
};


class Bullet {
public:
    Bullet(Vector3 position, Vector3 velocity, float lifetime, bool enemy,  BulletType t = BulletType::Default, float radius = 1.0f, bool launcher = false);
    BulletLight light;
    void Update(Camera& camera, float deltaTime);
    void UpdateMagicBall(Camera& camera, float deltaTime);
    void Erase();
    void Draw(Camera& camera) const;
    void kill(Camera& camera);
    bool IsExpired() const;
    bool IsAlive() const;
    bool IsEnemy() const;
    bool isFireball() const;
    bool isExploded() const;
    void Blood(Camera& camera);
    void Explode(Camera& camera);

    float GetRadius() const { return radius; }
    bool pendingExplosion = false;
    float explosionTimer = 0.0f;
    bool launcher = false;
    BulletType type = BulletType::Default;
    Vector3 GetPosition() const;

private:
    Emitter fireEmitter;
    Emitter sparkEmitter;
    Vector3 position;
    Vector3 velocity;   // replaces direction and speed
    bool alive;
    bool enemy;
    bool fireball;
    float age;
    float maxLifetime;
    float timer;
    float gravity = 300.0f;
    float radius = 1.5f;
    float spinAngle = 0.0f;
    bool exploded = false;
    float timeSinceExploded = 0.0f;
    bool explosionTriggered = false;
};

void FireBlunderbuss(Vector3 origin, Vector3 forward, float spreadDegrees, int pelletCount, float speed, float lifetime, bool enemy);
void FireBullet(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy);
void FireFireball(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy, bool launcher);
void FireIceball(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy);