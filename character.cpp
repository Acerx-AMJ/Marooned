#include "Character.h"
#include "raymath.h"
#include <iostream>
#include "resources.h"
#include "rlgl.h"
#include "world.h"
#include "algorithm"
//#include "world.h"


Character::Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row)
    : position(pos), texture(tex), frameWidth(fw), frameHeight(fh),
      currentFrame(0), maxFrames(frames), animationTimer(0), animationSpeed(speed),
      scale(scl), rowIndex(row) {}

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


void Character::Update(float deltaTime, Vector3 playerPosition, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors ) {
    animationTimer += deltaTime;
    stateTimer += deltaTime;
    previousPosition = position;
    float groundY = GetHeightAtWorldPosition(position, heightmap, terrainScale);

    // Gravity
    float gravity = 800.0f; 
    static float verticalVelocity = 0.0f;

    float spriteHeight = frameHeight * scale;

    if (position.y > groundY + spriteHeight / 2.0f) {
        verticalVelocity -= gravity * deltaTime;
        position.y += verticalVelocity * deltaTime;
    } else {
        verticalVelocity = 0.0f;
        position.y = groundY + spriteHeight / 2.0f;
    }
    float distance = Vector3Distance(position, playerPosition);
    float randomTime = GetRandomValue(1, 3);

    if (currentHealth <= 0) state = DinoState::Death;

    //idle, chase, attack, runaway
    switch (state) {
        case DinoState::Idle:
            if (distance < 3000.0f && stateTimer > 0.5f) { //if far enough away, stop. 
                state = DinoState::Chase;
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                hasRunawayAngle = false;
            }
            break;

        case DinoState::Chase: {
            if (distance < 150.0f) {
                state = DinoState::Attack;
                SetAnimation(2, 5, 0.1f);
                stateTimer = 0.0f;
            } else if (distance > 3000.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
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
                float spriteHeight = frameHeight * scale;

                //run away if near water. 
                if (currentTerrainHeight <= 65.0f) {
                    state = DinoState::RunAway;
                    SetAnimation(3, 4, 0.1f);
                    stateTimer = 0.0f;
                } else if (proposedTerrainHeight > 60.0f) {
                    position = proposedPosition;
                    rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                    position.y = GetHeightAtWorldPosition(position, heightmap, terrainScale) + (frameHeight * scale) / 2.0f; //recalculate height
                }

            }

            break;
        }



        case DinoState::Attack:
            if (distance > 200.0f) {
                state = DinoState::Chase;
                SetAnimation(1, 5, 0.12f);
            }
            
            if (stateTimer >= randomTime) {
                state = DinoState::RunAway;
                // On state change to RunAway
                runawayAngleOffset = DEG2RAD * GetRandomValue(-60, 60);
                hasRunawayAngle = true;
                SetAnimation(3, 4, 0.1f); // run away animation
                stateTimer = 0.0f;
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

            // If the current terrain is near water, force runaway state to continue,
            // but only move if the proposed position is on solid ground.
            if (proposedTerrainHeight > 60.0f) {
                position = proposedPos;
                position.y = proposedTerrainHeight + (frameHeight * scale) / 2.0f;
                rotationY = RAD2DEG * atan2f(awayDir.x, awayDir.z);
            }

            float randomDistance = GetRandomValue(1000, 2000);
            if (distance > randomDistance && stateTimer > 1.0f && currentTerrainHeight > 60.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
            }

            break;
        } 
        case DinoState::Death:
            if (!isDead) {
                SetAnimation(4, 4, 0.2f);  
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
            currentFrame = (currentFrame + 1) % maxFrames;
        }
    }


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

    rlDisableDepthMask();
    DrawBillboardRec(camera, *texture, sourceRec, offsetPos, size, WHITE);
    rlEnableDepthMask();
}


void Character::SetAnimation(int row, int frames, float speed) {
    rowIndex = row;
    maxFrames = frames;
    animationSpeed = speed;
    currentFrame = 0;
    animationTimer = 0;
}
