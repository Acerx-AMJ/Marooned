#pragma once
#include <vector>
#include "raylib.h"
#include "Particle.h"

class Emitter {
public:
    Emitter(); // <-- add this
    Emitter(Vector3 pos);

    void Update(float dt);
    void Draw(Camera3D camera) const;
    void SetPosition(Vector3 newPos);

private:
    Vector3 position;
    std::vector<Particle> particles;
    int maxParticles = 100;
    float emissionRate = 50.0f; // particles per second
    float timeSinceLastEmit = 0.0f;

    void EmitParticles(int count);
    Particle CreateParticle();
};
