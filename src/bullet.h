#pragma once

#include "raylib.h"
#include <vector>
#include "emitter.h"



class Bullet {
public:
    Bullet(Vector3 position, Vector3 velocity, float lifetime, bool enemy, bool fireball = false, float radius = 1.0f);

    void Update(Camera& camera, float deltaTime);
    void UpdateFireball(Camera& camera, float deltaTime);
    void SpawnBloodEffect(const Vector3& pos, Camera& camera);
    void Erase();
    void Draw(Camera& camera) const;
    void kill(Camera& camera);
    bool IsExpired() const;
    bool IsAlive() const;
    bool IsEnemy() const;
    bool isFireball() const;
    bool isExploded() const;
    void Blood(Camera camera);
    void BloodParticles();
    void Explode(Camera& camera);
    //void SetRadius(float r) { radius = r; } //inline, dont need to define in cpp
    float GetRadius() const { return radius; }


    Vector3 GetPosition() const;


    

private:
    Emitter fireEmitter;
    Emitter bloodEmitter;
    Vector3 position;
    Vector3 velocity;   // replaces direction and speed
    bool alive;
    bool enemy;
    bool fireball;
    
    float age;
    float maxLifetime;
    float timer;
    float gravity = 300.0f;
    float radius = 1.0f;
    float spinAngle = 0.0f;
    bool exploded = false;
    float timeSinceExploded = 0.0f;
    bool explosionTriggered = false;



    
};


void FireBlunderbuss(Vector3 origin, Vector3 forward, float spreadDegrees, int pelletCount, float speed, float lifetime, bool enemy);
void FireBullet(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy);
void FireFireball(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy, bool firball);
