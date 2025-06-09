#include "Character.h"
#include "raymath.h"
#include <iostream>
#include "resources.h"
#include "rlgl.h"
#include "world.h"
#include "algorithm"
#include "sound_manager.h"
#include "player.h"


Character::Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row)
    : position(pos),
      texture(tex),
      frameWidth(fw),
      frameHeight(fh),
      currentFrame(0),
      maxFrames(frames),
      rowIndex(row),
      animationTimer(0),
      animationSpeed(speed),
      scale(scl) {}

BoundingBox Character::GetBoundingBox() const {
    float halfWidth = (frameWidth * scale * 0.4f) / 2.0f;  // Only 1/3 of width
    float halfHeight = (frameHeight * scale) / 2.0f;

    return {
        { position.x - halfWidth, position.y - halfHeight, position.z - halfWidth },
        { position.x + halfWidth, position.y + halfHeight, position.z + halfWidth }
    };
}

void Character::playRaptorSounds(){

    int number = GetRandomValue(1, 3);
    //std::cout << "playing sound";
    switch (number)
    {
    case 1:
        SoundManager::GetInstance().PlaySoundAtPosition("dinoTweet", position, player.position, player.rotation.y, 8000);
        
        break;
    case 2:
        SoundManager::GetInstance().PlaySoundAtPosition("dinoTweet2", position, player.position, player.rotation.y, 8000);
        
        break;

    case 3:
        SoundManager::GetInstance().PlaySoundAtPosition("dinoTarget", position, player.position, player.rotation.y, 8000);
        break;
    } 

}


void Character::TakeDamage(int amount) {
    if (isDead) return;

    currentHealth -= amount;

    if (currentHealth <= 0) {
        currentHealth = 0;
        isDead = true;
        state = DinoState::Death;
        SetAnimation(4, 4, 0.2f); // your death anim here
        deathTimer = 0.0f;
        SoundManager::GetInstance().Play("dinoDeath");
        // Play death sound here if desired
    } else {
        hitTimer = 0.5f;
        state = DinoState::Stagger;
        SetAnimation(4, 1, 1.0f); // Use row 4, 1 frame, 1 second per frame
        currentFrame = 0;         // Always start at first frame
        stateTimer = 0.0f;

        SoundManager::GetInstance().Play("dinoHit");

    }
}



Vector3 Character::ComputeRepulsionForce(const std::vector<Character*>& allRaptors, float repulsionRadius, float repulsionStrength) {
    Vector3 repulsion = { 0 };

    for (Character* other : allRaptors) {
        if (other == this) continue;

        float dist = Vector3Distance(position, other->position);
        if (dist < repulsionRadius && dist > 1.0f) {
            Vector3 away = Vector3Normalize(Vector3Subtract(position, other->position)); // you - other
            float falloff = (repulsionRadius - dist) / repulsionRadius;
            repulsion = Vector3Add(repulsion, Vector3Scale(away, falloff * repulsionStrength));
        }
    }

    return repulsion;
}




void Character::Update(float deltaTime, Vector3 playerPosition, Player& player,  Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors ) {
    animationTimer += deltaTime;
    stateTimer += deltaTime;
    previousPosition = position;
    float groundY = GetHeightAtWorldPosition(position, heightmap, terrainScale);
    if (isDungeon) groundY = dungeonHeight;
    if (hitTimer > 0){
        hitTimer -= deltaTime;
    }else{
        hitTimer = 0;
    }
    // Gravity
    float gravity = 800.0f;
    if (isDungeon) gravity = 0.0f;
    static float verticalVelocity = 0.0f;

    float spriteHeight = frameHeight * scale;

    if (position.y > groundY + spriteHeight / 2.0f) {
        verticalVelocity -= gravity * deltaTime;
        position.y += verticalVelocity * deltaTime;
    } else {
        verticalVelocity = 0.0f;
        position.y = groundY + spriteHeight / 2.0f;
    }
    float distance = Vector3Distance(position, player.position);

    //idle, chase, attack, runaway, stagger, death

    //Raptors: roam around in the jungle untill they encounter player, chase player for 3-7 seconds, then run away, if close attack, if to far away roam.
    //raptors are afraid of water. If they are cornered by water, they will attack. 
    switch (state) {
        case DinoState::Idle:
            if (stateTimer > idleThreshold){ //if idle for 10 seconds run to a new spot. 
                state = DinoState::RunAway;
                randomTime = GetRandomValue(1,3);
                SetAnimation(3, 4, 0.1f);
                stateTimer = 0.0f;
                runawayAngleOffset = DEG2RAD * GetRandomValue(-180, 180);
                hasRunawayAngle = true;
                playRaptorSounds(); //make noises while they run around


            }

            if (distance < 4000.0f && stateTimer > 1.0f) { 
                state = DinoState::Chase; //switch to chase
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
                hasRunawayAngle = false;

                int number = GetRandomValue(1, 3);
                playRaptorSounds();

            }
            break;

        case DinoState::Chase: {
            stateTimer += deltaTime;

            if (distance < 150.0f) {
                //Gun is 100 away from player center, don't clip the gun. 
                state = DinoState::Attack;
                SetAnimation(2, 5, 0.1f);
                stateTimer = 0.0f;
                randomTime = GetRandomValue(1,3);
            } else if (distance > 4000.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
                idleThreshold = (float)GetRandomValue(5, 15);
            } else if (stateTimer >= chaseDuration) {
                // Give up and run away
                
                state = DinoState::RunAway;
                SetAnimation(3, 4, 0.1f);
                stateTimer = 0.0f;
                runawayAngleOffset = DEG2RAD * GetRandomValue(-50, 50); //run in a random direction
                randomDistance = GetRandomValue(1000, 2000); //set distance to run 
                if (isDungeon) randomDistance = GetRandomValue(500, 1000);
                hasRunawayAngle = true;
            } else {
                // Chase logic with repulsion
                Vector3 dir = Vector3Normalize(Vector3Subtract(playerPosition, position));
                Vector3 horizontalMove = { dir.x, 0, dir.z };

                // Add repulsion from other raptors
                Vector3 repulsion = ComputeRepulsionForce(allRaptors, 50, 500);
                Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

                Vector3 proposedPosition = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f)); //one step ahead. 

                float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPosition, heightmap, terrainScale); //see if the next step is water. 
                float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
                //float spriteHeight = frameHeight * scale;
                if (isDungeon) proposedTerrainHeight = dungeonHeight;
                if (isDungeon) currentTerrainHeight = dungeonHeight;
                //run away if near water. X Raptor now stops at waters edge, he no longer gets stuck though.
                if (currentTerrainHeight <= 65.0f && stateTimer > 1.0f) {
                    state = DinoState::RunAway;
                    SetAnimation(3, 4, 0.1f);
                    stateTimer = 0.0f;
                    randomDistance = GetRandomValue(1000, 2000); //set distance to run 
                    runawayAngleOffset = DEG2RAD * GetRandomValue(-50, 50); //run in a random direction
                    stateTimer = 0;

                } else if (proposedTerrainHeight > 60.0f) {
                    position = proposedPosition;
                    rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                    //dont recalculate height if in a dungeon. dungeon height stays the same. 
                    if (!isDungeon) position.y = GetHeightAtWorldPosition(position, heightmap, terrainScale) + (frameHeight * scale) / 2.0f; //recalculate height
                }

            }

            break;
        }
        case DinoState::Attack:
            if (distance > 200.0f) { //maybe this should be like 160, player would get bit more. 
                state = DinoState::Chase;
                SetAnimation(1, 5, 0.12f);
            }

            attackCooldown -= deltaTime;
            if (attackCooldown <= 0.0f) {
                attackCooldown = 1.0f; // seconds between attacks

                // Play attack sound
                SoundManager::GetInstance().Play("dinoBite");

                // Damage the player
                player.TakeDamage(10);
            }

            if (stateTimer >= randomTime) {
                state = DinoState::RunAway;
                runawayAngleOffset = DEG2RAD * GetRandomValue(-60, 60);
                hasRunawayAngle = true;
                SetAnimation(3, 4, 0.1f); // run away animation
                stateTimer = 0.0f;
                randomDistance = GetRandomValue(1000, 2000);
            }
            break;


        case DinoState::RunAway: {
            Vector3 awayDir = Vector3Normalize(Vector3Subtract(position, playerPosition)); // direction away from player

            if (!hasRunawayAngle) {
                runawayAngleOffset = DEG2RAD * GetRandomValue(-60, 60);
                hasRunawayAngle = true;
            }
            float baseAngle = atan2f(position.z - playerPosition.z, position.x - playerPosition.x);
            float finalAngle = baseAngle + runawayAngleOffset;

            Vector3 veerDir = { cosf(finalAngle), 0.0f, sinf(finalAngle) };

            Vector3 horizontalMove = { veerDir.x, 0, veerDir.z };

            // Add repulsion
            Vector3 repulsion = ComputeRepulsionForce(allRaptors, 50, 500);
            Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

            Vector3 proposedPos = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f));
            float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
            float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPos, heightmap, terrainScale);
            if (isDungeon) proposedTerrainHeight = dungeonHeight;
            if (isDungeon) currentTerrainHeight = dungeonHeight;
            // If the current terrain is near water, force runaway state to continue,
            // but only move if the proposed position is on solid ground.
            if (proposedTerrainHeight > 60.0f) {
                position = proposedPos;
                position.y = proposedTerrainHeight + (frameHeight * scale) / 2.0f;
                rotationY = RAD2DEG * atan2f(awayDir.x, awayDir.z);
            }

            
            if (distance > randomDistance && stateTimer > 1.0f && currentTerrainHeight > 60.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
                idleThreshold = (float)GetRandomValue(5, 15);
            }
            if (currentTerrainHeight <= 65.0f) {
                state = DinoState::Chase;
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                hasRunawayAngle = false;
                //break; // exit RunAway logic
            }
            break;
        }

        case DinoState::Stagger: {
            //do nothing
            if (stateTimer >= 0.6f) {
                state = DinoState::Chase;
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
            }
            break;
        }
        case DinoState::Death:
            if (!isDead) {
                SetAnimation(4, 5, 0.15f);  
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;

            break;
    }

    if (animationTimer >= animationSpeed) {
        animationTimer = 0;

        if (state == DinoState::Death) {
            if (currentFrame < maxFrames - 1) {
                currentFrame++;
            }
            // else do nothing — stay on last frame
        } else {
            currentFrame = (currentFrame + 1) % maxFrames; //loop
        }
    }

    //erase dead raptors from raptorPtrs 
    raptorPtrs.erase(std::remove_if(raptorPtrs.begin(), raptorPtrs.end(),
    [](Character* raptor) {
        return raptor->isDead && raptor->deathTimer > 5.0f;
    }),
    raptorPtrs.end());

}


void Character::Draw(Camera3D camera) {
    Rectangle sourceRec = {
        (float)(currentFrame * frameWidth),
        (float)(rowIndex * frameHeight),
        (float)frameWidth,
        (float)frameHeight
    };

    // Calculate a slight camera-facing offset to reduce z-fighting
    Vector3 camDir = Vector3Normalize(Vector3Subtract(camera.position, position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, 1.0f)); // Adjust 0.1f if needed

    Vector2 size = { frameWidth * scale, frameHeight * scale };
    //if (isDungeon) offsetPos.y += 0.1;
    Color dinoTint = (hitTimer > 0.0f) ? (Color){255, 50, 50, 255} : WHITE;
    rlDisableDepthMask();
    
    DrawBillboardRec(camera, *texture, sourceRec, offsetPos, size, dinoTint);
    //DrawBoundingBox(GetBoundingBox(), RED);
    //rlEnableBackfaceCulling();
    rlEnableDepthMask();
}


void Character::SetAnimation(int row, int frames, float speed) {
    rowIndex = row;
    maxFrames = frames;
    animationSpeed = speed;
    currentFrame = 0;
    animationTimer = 0;
}
