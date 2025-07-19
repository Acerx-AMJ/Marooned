#include "Emitter.h"

Emitter::Emitter()
    : position({0, 0, 0}) // Or something like {0, -9999, 0} to keep it hidden
{
    particles.resize(maxParticles);
}

Emitter::Emitter(Vector3 pos) : position(pos) {
    particles.resize(maxParticles);
}

void Emitter::SetPosition(Vector3 newPos) {
    position = newPos;
}

void Emitter::Update(float dt) {
    timeSinceLastEmit += dt;

    int toEmit = static_cast<int>(emissionRate * timeSinceLastEmit);
    if (toEmit > 0) {
        EmitParticles(toEmit);
        timeSinceLastEmit = 0.0f;
    }

    for (auto& p : particles) {
        p.Update(dt);
    }
}

void Emitter::EmitParticles(int count) {
    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (!p.active) {
                
                p = CreateParticle();
                break;
            }
        }
    }
}

Particle Emitter::CreateParticle() {
    Particle p;
    p.active = true;
    p.position = position;

    // Basic random spray pattern
    p.velocity = {
        (float)(GetRandomValue(-100, 100) / 100.0f),
        (float)(GetRandomValue(50, 150) / 100.0f),
        (float)(GetRandomValue(-100, 100) / 100.0f)
    };

  

    p.maxLife = 1.5f;
    p.life = p.maxLife;
    p.color = RED;
    p.size = 4.0f;

    return p;
}

void Emitter::Draw(Camera3D camera) const{
    for (const auto& p : particles) {
        p.Draw(camera);
        
    }
}
