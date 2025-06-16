#include "Character.h"
#include "raymath.h"
#include <iostream>
#include "resources.h"
#include "rlgl.h"
#include "world.h"
#include "algorithm"
#include "sound_manager.h"
#include "player.h"
#include "dungeonGeneration.h"
#include "pathfinding.h"


Character::Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row, CharacterType t)
    : position(pos),
      texture(tex),
      frameWidth(fw),
      frameHeight(fh),
      currentFrame(0),
      maxFrames(frames),
      rowIndex(row),
      animationTimer(0),
      animationSpeed(speed),
      scale(scl),
      type(t) {}



void Character::UpdateCollider() {
    float height = 150.0f; // or whatever your enemy height is
    float width = radius;

    Vector3 min = {
        position.x - width,
        position.y - height/2,
        position.z - width
    };

    Vector3 max = {
        position.x + width,
        position.y + height,
        position.z + width
    };

    collider = { min, max };
}
      
void Character::setPath(){
    Vector2 start = {
        (float)GetDungeonImageX(position.x, tileSize, dungeonWidth),
        (float)GetDungeonImageY(position.z, tileSize, dungeonHeight)
    };

    Vector2 goal = {
        (float)GetDungeonImageX(player.position.x, tileSize, dungeonWidth),
        (float)GetDungeonImageY(player.position.z, tileSize, dungeonHeight)
    };

    // Optional: don't recalculate unless player tile has changed
    if (currentPath.empty() || goal != currentPath.back()) {
        currentPath = FindPath(start, goal);
    }

}

void Character::UpdateRaptorAI(float deltaTime, Player& player, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors) {
    
    float distance = Vector3Distance(position, player.position);
    playerVisible = false;


    if (!isDungeon && distance < 4000){
        playerVisible = true;
    }


    //idle, chase, attack, runaway, stagger, death
    //Raptors: roam around in the jungle untill they encounter player, chase player for 3-7 seconds, then run away, if close attack, if to far away roam.
    //raptors are afraid of water. If they are cornered by water, they will attack. 
    switch (state) {
        case DinoState::Idle:
            if (stateTimer > idleThreshold){ //if idle for 10 seconds run to a new spot. 
                state = DinoState::RunAway;
                randomTime = GetRandomValue(5,15);
                SetAnimation(3, 4, 0.1f);//row 3, 4 frames, 0.1 frametime
                stateTimer = 0.0f;
                runawayAngleOffset = DEG2RAD * GetRandomValue(-180, 180);
                hasRunawayAngle = true;
                //playRaptorSounds(); //make noises while they run around
            }

            if (distance < 4000.0f && stateTimer > 1.0f) { 

                //if (isDungeon) setPath();
                state = DinoState::Chase; //switch to chase
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
                hasRunawayAngle = false;

                //int number = GetRandomValue(1, 3);
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
                randomTime = GetRandomValue(5,15);
            } else if (distance > 4000.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
                idleThreshold = (float)GetRandomValue(5,15);
            } else if (stateTimer >= chaseDuration && !isDungeon) {
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
                Vector3 dir = Vector3Normalize(Vector3Subtract(player.position, position));
                Vector3 horizontalMove = { dir.x, 0, dir.z };

                // Add repulsion from other raptors
                Vector3 repulsion = ComputeRepulsionForce(allRaptors, 50, 500);
                Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

                Vector3 proposedPosition = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f)); //one step ahead. 

                float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPosition, heightmap, terrainScale); //see if the next step is water. 
                float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
                //float spriteHeight = frameHeight * scale;
                if (isDungeon) proposedTerrainHeight = dungeonPlayerHeight;
                if (isDungeon) currentTerrainHeight = dungeonPlayerHeight;
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
            Vector3 awayDir = Vector3Normalize(Vector3Subtract(position, player.position)); // direction away from player

            if (!hasRunawayAngle) {
                runawayAngleOffset = DEG2RAD * GetRandomValue(-60, 60);
                hasRunawayAngle = true;
            }
            float baseAngle = atan2f(position.z - player.position.z, position.x - player.position.x);
            float finalAngle = baseAngle + runawayAngleOffset;

            Vector3 veerDir = { cosf(finalAngle), 0.0f, sinf(finalAngle) };

            Vector3 horizontalMove = { veerDir.x, 0, veerDir.z };

            // Add repulsion
            Vector3 repulsion = ComputeRepulsionForce(allRaptors, 50, 500);
            Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

            Vector3 proposedPos = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f));
            float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
            float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPos, heightmap, terrainScale);
            if (isDungeon) proposedTerrainHeight = dungeonPlayerHeight;
            if (isDungeon) currentTerrainHeight = dungeonPlayerHeight;
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
                idleThreshold = (float)GetRandomValue(1, 5);
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
            stateTimer += deltaTime;
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
    }




void Character::UpdateAI(float deltaTime, Player& player, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors) {
    switch (type) {
        case CharacterType::Raptor:
            UpdateRaptorAI(deltaTime, player, heightmap, terrainScale, allRaptors);
            break;
        case CharacterType::Skeleton:
            UpdateSkeletonAI(deltaTime, player, allRaptors);
            break;
    }
}

void Character::UpdateSkeletonAI(float deltaTime, Player& player, const std::vector<Character*>& allRaptors) {

    static float repathTimer = 0.0f;
    repathTimer += deltaTime;

    const float repathInterval = 0.4f; // seconds between path recalculations

    float distance = Vector3Distance(position, player.position);
    playerVisible = false;
    if (isDungeon){

        Vector2 start = WorldToImageCoords(position);
        Vector2 goal = WorldToImageCoords(player.position);

        bool canSee = LineOfSightRaycast(start, goal, dungeonImg, 30);

        if (canSee) {
            playerVisible = true;
            timeSinceLastSeen = 0.0f;
        } else {
            timeSinceLastSeen += deltaTime;
            if (timeSinceLastSeen > forgetTime) {
                playerVisible = false;
            }
        }

    }
 
    switch (state){
        case DinoState::Idle:
            if (distance < 4000.0f && stateTimer > 1.0f && playerVisible) {
                state = DinoState::Chase;
                SetAnimation(1, 4, 0.25f);
                stateTimer = 0.0f;

                Vector2 start = WorldToImageCoords(position);
                Vector2 goal = WorldToImageCoords(player.position);

                std::vector<Vector2> tilePath = SmoothPath(FindPath(start, goal), dungeonImg);
                currentWorldPath.clear();
                for (const Vector2& tile : tilePath) {
                    Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                    worldPos.y += 80.0f;
                    currentWorldPath.push_back(worldPos);
                }
            }

            else if (stateTimer > 10.0f) { // ← idle too long? pick a new destination
                                // Try random reachable tile in dungeon
                Vector2 start = WorldToImageCoords(position);
                Vector2 randomTile;
                bool found = false;

                int patrolRadius = 3;

                for (int i = 0; i < 10; ++i) { // try 10 random nearby tiles
                    int rx = (int)start.x + GetRandomValue(-patrolRadius, patrolRadius);
                    int ry = (int)start.y + GetRandomValue(-patrolRadius, patrolRadius);

                    if (rx < 0 || ry < 0 || rx >= dungeonWidth || ry >= dungeonHeight)
                        continue;

                    if (walkable[rx][ry] && !IsTileOccupied(rx, ry, skeletonPtrs, this)) {
                        randomTile = {(float)rx, (float)ry};
                        found = true;
                        break;
                    }
                }

                if (found) {
                    std::vector<Vector2> tilePath = FindPath(start, randomTile);
                    
                    currentWorldPath.clear();

                    for (const Vector2& tile : tilePath) {
                        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                        worldPos.y += 80.0f;
                        currentWorldPath.push_back(worldPos);
                    }

                    if (!currentWorldPath.empty()) {
                        state = DinoState::Patrol;
                        SetAnimation(1, 2, 0.12f); // reuse walk anim
                    }
                }
    }
            break;

        
        case DinoState::Chase:
            stateTimer += deltaTime;
            pathCooldownTimer -= deltaTime;

            if (distance < 150.0f) {
                state = DinoState::Attack;
                SetAnimation(2, 4, 0.2f);
                stateTimer = 0.0f;
                attackCooldown = 0.0f;

            } else if (distance > 4000.0f) {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;

            } else {
                Vector2 currentPlayerTile = WorldToImageCoords(player.position);
                if (((int)currentPlayerTile.x != (int)lastPlayerTile.x || (int)currentPlayerTile.y != (int)lastPlayerTile.y)
                    && pathCooldownTimer <= 0.0f) {

                    lastPlayerTile = currentPlayerTile; //save the last player tile so we aren't recalculating if the player hasn't moved. 
                    pathCooldownTimer = 0.4f;

                    Vector2 start = WorldToImageCoords(position);
                    //std::vector<Vector2> tilePath = FindPath(start, currentPlayerTile);
                    std::vector<Vector2> tilePath = SmoothPath(FindPath(start, currentPlayerTile), dungeonImg);
                    currentWorldPath.clear();
                    for (const Vector2& tile : tilePath) {
                        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                        worldPos.y += 80.0f;
                        currentWorldPath.push_back(worldPos);
                    }
                }

                // 🧭 Move along current path
                if (!currentWorldPath.empty()) {
                    Vector3 targetPos = currentWorldPath[0];
                    Vector3 dir = Vector3Normalize(Vector3Subtract(targetPos, position));
                    Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                    position = Vector3Add(position, move);
                    rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                    position.y = targetPos.y;

                    if (Vector3Distance(position, targetPos) < 100.0f) {
                        currentWorldPath.erase(currentWorldPath.begin());
                    }
                }
            }
            break;



        case DinoState::Attack: {
            //dont stand on the same tile as another skele when attacking
            Vector2 myTile = WorldToImageCoords(position);
            Character* occupier = GetTileOccupier(myTile.x, myTile.y, skeletonPtrs, this);

            if (occupier && occupier != this) {
                // Only the one with the "greater" pointer backs off
                if (this > occupier) {
                    state = DinoState::Reposition;
                    SetAnimation(1, 4, 0.12f);
                    stateTimer = 0.0f;
                    break;
                } else {
                    // Let the other one reposition — wait
                    break;
                }
            }

            if (distance > 200.0f) { //maybe this should be like 160, player would get bit more. 
                state = DinoState::Chase;
                SetAnimation(1, 4, 0.25f);
                stateTimer = 0.0;
            }



            attackCooldown -= deltaTime;
            if (attackCooldown <= 0.0f) {
                attackCooldown = 1.5f; // seconds between attacks

                // Play attack sound
                SoundManager::GetInstance().Play("dinoBite");

                // Damage the player
                player.TakeDamage(10);
            }
            break;
        }
        case DinoState::Reposition: {
            
            const int offsets[8][2] = {
                {  1,  0 }, { -1,  0 }, { 0,  1 }, { 0, -1 },
                {  1,  1 }, { -1, -1 }, { 1, -1 }, { -1, 1 }
            };

            Vector2 myTile = WorldToImageCoords(position);

            for (int i = 0; i < 8; ++i) {
                int tx = (int)myTile.x + offsets[i][0];
                int ty = (int)myTile.y + offsets[i][1];

                if (tx < 0 || ty < 0 || tx >= dungeonWidth || ty >= dungeonHeight) continue;
                if (!IsWalkable(tx, ty)) continue;
                if (IsTileOccupied(tx, ty, skeletonPtrs, this)) continue;

                // Found a free spot
                Vector3 target = GetDungeonWorldPos(tx, ty, tileSize, dungeonPlayerHeight);
                target.y += 80.0f;

                Vector3 dir = Vector3Normalize(Vector3Subtract(target, position));
                Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                position = Vector3Add(position, move);

                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = target.y;

                float dist = Vector3Distance(position, target);

                if (dist < 150.0f) {
                    state = DinoState::Attack;
                    SetAnimation(2, 5, 0.2f);
                } else if (dist > 200.0f) {
                    state = DinoState::Chase;
                    SetAnimation(1, 4, 0.25f);
                }

                break;
            }

            break;
        }

        case DinoState::Patrol: {
            stateTimer += deltaTime;

            if (distance < 4000 && playerVisible){
                state = DinoState::Chase;
                SetAnimation(1, 4, 0.25f);
            }

            if (!currentWorldPath.empty()) {
                Vector3 targetPos = currentWorldPath[0];
                Vector3 dir = Vector3Normalize(Vector3Subtract(targetPos, position));
                Vector3 move = Vector3Scale(dir, 500 * deltaTime); // maybe slower than chase
                position = Vector3Add(position, move);
                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = targetPos.y;

                if (Vector3Distance(position, targetPos) < 100.0f) {
                    currentWorldPath.erase(currentWorldPath.begin());
                }
            }
            else {
                state = DinoState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
            }

            break;
        }


        case DinoState::Stagger: {
            stateTimer += deltaTime;
            //do nothing
            if (stateTimer >= 0.6f) {
                state = DinoState::Chase;
                SetAnimation(1, 4, 0.25f);
                stateTimer = 0.0f;
                //chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
            }
            break;
        }

        case DinoState::Death:
            if (!isDead) {
                SetAnimation(4, 3, 0.5f);  
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;
            break;
        }
       
}



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
        if (type == CharacterType::Raptor) SetAnimation(4, 5, 0.12f);
        if (type == CharacterType::Skeleton) SetAnimation(4, 3, 0.5f); //less froms for skele death. 
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

void Character::eraseCharacters(){
    //erase dead raptors from raptorPtrs 
    raptorPtrs.erase(std::remove_if(raptorPtrs.begin(), raptorPtrs.end(),
    [](Character* raptor) {
        return raptor->isDead && raptor->deathTimer > 5.0f;
    }),
    raptorPtrs.end());

    skeletonPtrs.erase(std::remove_if(skeletonPtrs.begin(), skeletonPtrs.end(),
    [](Character* skeleton) {
        return skeleton->isDead && skeleton->deathTimer > 5.0f;
    }),
    skeletonPtrs.end());
}


void Character::Update(float deltaTime, Player& player,  Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors ) {
    animationTimer += deltaTime;
    stateTimer += deltaTime;
    previousPosition = position;
    float groundY = GetHeightAtWorldPosition(position, heightmap, terrainScale);
    if (isDungeon) groundY = dungeonPlayerHeight;
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
    
   
    UpdateAI(deltaTime,player, heightmap, terrainScale, allRaptors);
    UpdateCollider();


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

    eraseCharacters(); //clean up dead rators and skeletons

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
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, 10.0f)); // Adjust 0.1f if needed
    if (type == CharacterType::Skeleton) scale = 0.8;
    Vector2 size = { frameWidth * scale, frameHeight * scale };

    Color dinoTint = (hitTimer > 0.0f) ? (Color){255, 50, 50, 255} : WHITE;
    rlDisableDepthMask();
   //DrawBoundingBox(collider, RED);
    DrawBillboardRec(camera, *texture, sourceRec, offsetPos, size, dinoTint);

    rlEnableDepthMask();
}


void Character::SetAnimation(int row, int frames, float speed) {
    rowIndex = row;
    maxFrames = frames;
    animationSpeed = speed;
    currentFrame = 0;
    animationTimer = 0;
}
