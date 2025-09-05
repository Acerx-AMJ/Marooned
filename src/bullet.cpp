#include "bullet.h"
#include "raymath.h"
#include <raylib.h>
#include <cmath>
#include "world.h"
#include "resourceManager.h"
#include "decal.h"
#include "sound_manager.h"
#include "utilities.h"
#include "dungeonGeneration.h"


// New constructor matching velocity-based logic
Bullet::Bullet(Vector3 startPos, Vector3 vel, float lifetime, bool en, BulletType t, float r, bool launch)
    : position(startPos),
      velocity(vel),
      alive(true),
      age(0.0f),
      maxLifetime(lifetime),
      enemy(en),
      type(t),
      fireEmitter(startPos),
      sparkEmitter(startPos),
      radius(r),
      launcher(launch)
{}


void Bullet::UpdateMagicBall(Camera& camera, float deltaTime) {
    if (!alive) return;

    // Gravity-based arc
    
    gravity = launcher ? 0 : 980; //fireballs fired from traps have no gravity
    fireEmitter.SetPosition(position);
    sparkEmitter.SetPosition(position);
    
    velocity.y -= gravity * deltaTime;

    // Move
    position = Vector3Add(position, Vector3Scale(velocity, deltaTime));

    spinAngle += 90.0f * deltaTime; // 90 degrees per second
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    // Collision with floor
    if (isDungeon) {
        if (position.y <= dungeonPlayerHeight) {
            //Explode(camera);
            Explode(camera);

            return;
        }
        if (position.y >= ceilingHeight) {
            Explode(camera);
            
            return;
        }
    } else {
        float terrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
        if (position.y <= terrainHeight) {
            Explode(camera);
            return;
        }
    }

    if (pendingExplosion) {
        explosionTimer -= deltaTime;
        if (explosionTimer <= 0.0f) {
            Explode(camera); // now do the actual explosion logic
            pendingExplosion = false;
        }
    }

    // Lifetime kill
    age += deltaTime;
    if (age >= maxLifetime && type == BulletType::Default) {
        kill(camera);
    }



}



void Bullet::Update(Camera& camera, float deltaTime) {

    if (!alive) return;

    // Fireball logic
    if (type == BulletType::Fireball) {
        
        fireEmitter.SetParticleType(ParticleType::Smoke);
        fireEmitter.Update(deltaTime);
        sparkEmitter.SetParticleType(ParticleType::FireTrail);
        sparkEmitter.Update(deltaTime);
        UpdateMagicBall(camera, deltaTime);

        if (!exploded && explosionTriggered) {
            exploded = true;
     
        }
        
        if (exploded){
            timeSinceExploded += deltaTime;

            if (timeSinceExploded >= 2.0f) { //wait for particles to act. 
                alive = false;
                return;

            }

        }
        return; //skip normal bullet logic
    }
    else if (type == BulletType::Iceball){
        sparkEmitter.SetParticleType(ParticleType::IceMist);
        fireEmitter.SetParticleType(ParticleType::IceMist);
        fireEmitter.Update(deltaTime);
        sparkEmitter.Update(deltaTime); 
        UpdateMagicBall(camera, deltaTime);

        if (!exploded && explosionTriggered) {
            exploded = true;
     
        }
        
        if (exploded){
            timeSinceExploded += deltaTime;

            if (timeSinceExploded >= 2.0f) { //wait for particles to act. 
                alive = false;
                return;

            }

        }
        return; //skip normal bullet logic
    }


    // Standard bullet movement (non-fireball)
    if (!IsEnemy() && type == BulletType::Default)
        velocity.y -= gravity * deltaTime;

    position = Vector3Add(position, Vector3Scale(velocity, deltaTime));
    age += deltaTime;

    if (age >= maxLifetime && !exploded) alive = false;



    // Handle floor/ceiling collision
    if (isDungeon) {
        if (position.y <= dungeonPlayerHeight || position.y >= ceilingHeight)
            kill(camera);
    } else {
        float terrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
        if (position.y <= terrainHeight)
            kill(camera);
    }
}



void Bullet::Draw(Camera& camera) const {
    if (!alive) return;
    if (type == BulletType::Fireball){
        fireEmitter.Draw(camera); //explosion particles 
        //bullet remains alive until timeSinceExplosion = 2.0f, always draw explosion particles.  
        
        if (!exploded){
            //dont draw the ball or firetrail if it's exploded. 
            DrawModelEx(R.GetModel("fireballModel"), position, { 0, 1, 0 }, spinAngle, { 20.0f, 20.0f, 20.0f }, WHITE);
            sparkEmitter.Draw(camera); //firetrail
        } 
        
    }else if (type == BulletType::Iceball){
        fireEmitter.Draw(camera);

        if (!exploded){
            DrawModelEx(R.GetModel("iceballModel"), position, { 0, 1, 0 }, spinAngle, { 25.0f, 25.0f, 25.0f }, WHITE);
            sparkEmitter.Draw(camera);
            
        } 
    } else{
        DrawSphere(position, 1.5f, WHITE); 
    }
}

bool Bullet::IsExpired() const {
    return alive;
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

bool Bullet::isExploded() const {
    return exploded;
}

Vector3 Bullet::GetPosition() const {
    return position;
}

void Bullet::Erase(){
    alive = false;
}

void Bullet::kill(Camera& camera){
    //smoke decals and bullet death
    Vector3 camDir = Vector3Normalize(Vector3Subtract(position, camera.position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, -100.0f));

    decals.emplace_back(offsetPos, DecalType::Smoke, R.GetTexture("smokeSheet"), 7, 0.8f, 0.1f, 25.0f);

    alive = false;
    
}



void Bullet::Blood(Camera& camera){
    //spawn blood decals at bullet position, if it's not a skeleton.
    Vector3 camDir = Vector3Normalize(Vector3Subtract(position, camera.position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, -100.0f));

    decals.emplace_back(offsetPos, DecalType::Blood, R.GetTexture("bloodSheet"), 7, 1.0f, 0.1f, 60.0f);

    alive = false;

}


void Bullet::Explode(Camera& camera) {
    if (!alive) return; 
    if (type == BulletType::Default) return; //we were calling explode on default bullets some how. likely recycled bullets didn't get reset to defaults

    if (!explosionTriggered){
        explosionTriggered = true;
        velocity.x = 0; //stop bullets velocity when exploding but keep gravity. could bullet apply damage more than once if it remains alive?
        velocity.z = 0;
        SoundManager::GetInstance().PlaySoundAtPosition("explosion", position, player.position, player.rotation.y, 3000.0f);
        
        Vector3 camDir = Vector3Normalize(Vector3Subtract(position, camera.position));
        Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, -100.0f));
        if (type == BulletType::Fireball){
            decals.emplace_back(offsetPos, DecalType::Explosion, R.GetTexture("explosionSheet"), 13, 1.0f, 0.1f, 500.0f);
            fireEmitter.EmitBurst(position, 200, ParticleType::Sparks);

        }else if (type == BulletType::Iceball){
            
            fireEmitter.EmitBurst(position, 200, ParticleType::IceBlast);
        }

        float minDamage = 10;
        float maxDamage = 200;
        float explosionRadius = 200;
        //area damage
        if (type == BulletType::Fireball){
            for (Character* enemy : enemyPtrs){
                float dist = Vector3Distance(position, enemy->position);
                if (dist < explosionRadius) {
                    float dmg =  Lerp(maxDamage, minDamage, dist / explosionRadius);
                    enemy->TakeDamage(dmg);
                    
                }
            }
            //damage player if too close, 
            float pDamage = 50.0f;
            float pdist = Vector3Distance(player.position, position);
            if (pdist < explosionRadius){
                float dmg =  Lerp(pDamage, minDamage, pdist / explosionRadius);
                player.TakeDamage(dmg);
            }

        }else if (type == BulletType::Iceball){
            for (Character* enemy : enemyPtrs){
                float dist = Vector3Distance(position, enemy->position);
                if (dist < explosionRadius) {
                    float dmg =  Lerp(maxDamage, minDamage, dist / explosionRadius);
                    enemy->ChangeState(CharacterState::Freeze);
                    // enemy->state = CharacterState::Freeze;
                    // enemy->stateTimer = 0; 
                    // enemy->SetAnimation(0, 1, 1.0f); //shouldn't we call changeState(CharacterState::Freeze)?
                    enemy->currentHealth -= dmg; //dont call take damage, it triggers stagger which over rides freeze. 

                    

                }
            }

        }

        


    }

}




void FireBlunderbuss(Vector3 origin, Vector3 forward, float spreadDegrees, int pelletCount, float speed, float lifetime, bool enemy) {
    for (int i = 0; i < pelletCount; ++i) {
        // Convert spread from degrees to radians
        float spreadRad = spreadDegrees * DEG2RAD;

        // Random horizontal and vertical spread
        float yaw   = RandomFloat(-spreadRad, spreadRad);
        float pitch = RandomFloat(-spreadRad, spreadRad);


        // Create rotation matrix from yaw and pitch
        Matrix rotYaw = MatrixRotateY(yaw);
        Matrix rotPitch = MatrixRotate(Vector3CrossProduct(forward, { 0, 1, 0 }), pitch);
        Matrix spreadMatrix = MatrixMultiply(rotYaw, rotPitch);

        Vector3 dir = Vector3Transform(forward, spreadMatrix);
        dir = Vector3Normalize(dir);

        Vector3 velocity = Vector3Scale(dir, speed);
        activeBullets.emplace_back(origin, velocity, lifetime, enemy, BulletType::Default);

    }
}

void FireBullet(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy) {
    Vector3 direction = Vector3Subtract(target, origin);
    direction = Vector3Normalize(direction);
    Vector3 velocity = Vector3Scale(direction, speed);

    activeBullets.emplace_back(origin, velocity, lifetime, enemy);
}

void FireFireball(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy, bool launcher) {
    Vector3 direction = Vector3Normalize(Vector3Subtract(target, origin));
    Vector3 velocity = Vector3Scale(direction, speed);

    Bullet& b = activeBullets.emplace_back(origin, velocity, lifetime, enemy, BulletType::Fireball, 20.0f, launcher);

    b.light.active     = true;
    b.light.color      = (b.type==BulletType::Fireball)? Vector3{0.99f,0.0f,0.0f} : Vector3{0.0f,0.7f,0.9f};
    b.light.range      = 400.0f;
    b.light.intensity  = 0.5;
    b.light.detachOnDeath = true;
    b.light.lifeTime   = 0.25f; // short glow after death

    SoundManager::GetInstance().PlaySoundAtPosition((rand() % 2 == 0 ? "flame1" : "flame2"), origin, player.position, 0.0f, 3000.0f);


}

void FireIceball(Vector3 origin, Vector3 target, float speed, float lifetime, bool enemy) {
    Vector3 direction = Vector3Normalize(Vector3Subtract(target, origin));
    Vector3 velocity = Vector3Scale(direction, speed);

    Bullet& b = activeBullets.emplace_back(origin, velocity, lifetime, enemy, BulletType::Iceball, 20.0f);

    b.light.active     = true;
    b.light.color      = (b.type==BulletType::Fireball)? Vector3{1.0f,0.15f,0.0f} : Vector3{0.0f,0.7f,0.9f};
    b.light.range      = 500.0f;
    b.light.intensity  = 0.75;
    b.light.detachOnDeath = true;
    b.light.lifeTime   = 0.25f; // short glow after death

    SoundManager::GetInstance().Play("iceMagic");
}

