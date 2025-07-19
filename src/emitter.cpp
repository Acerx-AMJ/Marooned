#include "Emitter.h"
#include <iostream>

Emitter::Emitter()
    : position({0, -9999, 0}) // Or something like {0, -9999, 0} to keep it hidden
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
    
    if (emissionRate > 0.0f) {
        timeSinceLastEmit += dt;

        int toEmit = static_cast<int>(emissionRate * timeSinceLastEmit);
        if (toEmit > 0) {
            EmitParticles(toEmit);
            timeSinceLastEmit = 0.0f;
        }
    }

    for (auto& p : particles) {
        p.Update(dt);
    }
}

void Emitter::UpdateBlood(float dt){
  for (auto& p : particles) {
        p.Update(dt);
    }
}

void Emitter::EmitBurst(Vector3 pos, int count) {
    if (canBurst) {
        canBurst = false;
        SetPosition(pos);

        this->isExplosionBurst = true; // Store the flag
        EmitParticles(count);
        this->isExplosionBurst = false; // Reset after emitting
    }
}

void Emitter::EmitBlood(Vector3 pos, int count) {
    SetPosition(pos);

    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (!p.active) {
                p.active = true;
                p.position = position;

                p.color = RED;
                p.gavity = 200.0f;  
                p.velocity = {
                    (float)GetRandomValue(-100, 100),
                    (float)GetRandomValue(50, 150),
                    (float)GetRandomValue(-100, 100)
                };

                p.maxLife = 1.0f;
                p.life = p.maxLife;
                p.size = 2.5f;

                break;
            }
        }
    }
}


void Emitter::EmitParticles(int count) {
    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (!p.active) {
                CreateParticle(p); // ← reuse particle

                break;
            }
        }
    }
}

void Emitter::CreateParticle(Particle& p) {
    p.active = true;
    p.position = position;

    if (isExplosionBurst) {
        p.color = ORANGE;
        p.gavity = 980.0f;
        p.velocity = {
            (float)(GetRandomValue(-300, 300)),
            (float)(GetRandomValue(300, 1000)),
            (float)(GetRandomValue(-300, 300))
        };
    } else {
        p.color = DARKGRAY;
        p.gavity = -100.0f;
        p.velocity = {
            (float)(GetRandomValue(-50, 50)),
            (float)(GetRandomValue(-25, 25)),
            (float)(GetRandomValue(-50, 50))
        };
    }

    p.maxLife = 1.5f;
    p.life = p.maxLife;
    p.size = 4.0f;
}




void Emitter::Draw(Camera3D camera) const{
    for (const auto& p : particles) {
        p.Draw(camera);
        
    }
}
