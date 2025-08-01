#pragma once
#include <vector>
#include "raylib.h"
#include "Particle.h"

enum class ParticleType {
    Smoke,
    Sparks,
    Blood,
    IceMist,
    IceBlast,

};


class Emitter {
public:
    Emitter(); // <-- add this
    Emitter(Vector3 pos);
    ParticleType particleType = ParticleType::Smoke; //default
    void EmitBurst(Vector3 pos, int count, ParticleType t);
    void EmitBlood(Vector3 pos, int count, Color color);
    void UpdateBlood(float dt);
    void Update(float dt); //update smoke and fire
    void Draw(Camera3D camera) const;
    void SetPosition(Vector3 newPos);
    
    void SetParticleType(ParticleType type) { particleType = type; }

private:
    Vector3 position;
    std::vector<Particle> particles;
    int maxParticles = 1000;
    float emissionRate = 100.0f; // particles per second
    float timeSinceLastEmit = 0.0f;
    bool isExplosionBurst = false;
    bool canBurst = true;
    void EmitParticles(int count);
    void CreateParticle(Particle& p);
};
