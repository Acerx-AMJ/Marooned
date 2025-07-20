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


void Character::UpdateAI(float deltaTime, Player& player,const Image& heightmap, Vector3 terrainScale) {
    switch (type) {
        case CharacterType::Raptor:
            UpdateRaptorAI(deltaTime, player, heightmap, terrainScale);
            break;
        case CharacterType::Skeleton:
            UpdateSkeletonAI(deltaTime, player);
            break;

        case CharacterType::Pirate:
            UpdatePirateAI(deltaTime, player);
            break;

        case CharacterType::Spider:
            UpdateSkeletonAI(deltaTime, player); //spider uses same code as skeleton
            break;
    }
}



void Character::UpdateRaptorAI(float deltaTime, Player& player,const Image& heightmap, Vector3 terrainScale) {
    if (isLoadingLevel) return;
    float distance = Vector3Distance(position, player.position);
    playerVisible = false;

    if (!isDungeon && distance < 4000){
        playerVisible = true; //player is always visible to raptors within range. 
    }

    //idle, chase, attack, runaway, stagger, death
    //Raptors: roam around in the jungle untill they encounter player, chase player for 3-7 seconds, then run away, if close attack, if to far away roam.
    //raptors are afraid of water. If they are cornered by water, they will attack. 
    switch (state) {
        case CharacterState::Idle:
            if (stateTimer > idleThreshold){ //if idle for 10 seconds run to a new spot. 
                state = CharacterState::RunAway;
                randomTime = GetRandomValue(5,15);
                SetAnimation(3, 4, 0.1f);//row 3, 4 frames, 0.1 frametime
                stateTimer = 0.0f;
                runawayAngleOffset = DEG2RAD * GetRandomValue(-180, 180);
                hasRunawayAngle = true;
                //playRaptorSounds(); //make noises while they run around
            }

            if (distance < 4000.0f && stateTimer > 1.0f) { 

                //if (isDungeon) setPath();
                state = CharacterState::Chase; //switch to chase
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
                hasRunawayAngle = false;

                //int number = GetRandomValue(1, 3);
                playRaptorSounds();

            }
            break;

        case CharacterState::Chase: {
            stateTimer += deltaTime;

            if (distance < 150.0f) {
                //Gun is 100 away from player center, don't clip the gun. 
                state = CharacterState::Attack;
                SetAnimation(2, 5, 0.1f);
                stateTimer = 0.0f;
                randomTime = GetRandomValue(5,15);
            } else if (distance > 4000.0f) {
                state = CharacterState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
                idleThreshold = (float)GetRandomValue(5,15);
            } else if (stateTimer >= chaseDuration && !isDungeon) {
                // Give up and run away
                
                state = CharacterState::RunAway;
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
                Vector3 repulsion = ComputeRepulsionForce(enemyPtrs, 50, 500);
                Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

                Vector3 proposedPosition = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f)); //one step ahead. 

                float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPosition, heightmap, terrainScale); //see if the next step is water. 
                float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
                //float spriteHeight = frameHeight * scale;
                if (isDungeon) proposedTerrainHeight = dungeonPlayerHeight;
                if (isDungeon) currentTerrainHeight = dungeonPlayerHeight;
                //run away if near water. X Raptor now stops at waters edge, he no longer gets stuck though.
                if (currentTerrainHeight <= 65.0f && stateTimer > 1.0f) {
                    state = CharacterState::RunAway;
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
        case CharacterState::Attack:
            if (distance > 200.0f) { //maybe this should be like 160, player would get bit more. 
                state = CharacterState::Chase;
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
            }

            attackCooldown -= deltaTime;
            if (attackCooldown <= 0.0f) {
                attackCooldown = 1.0f; // seconds between attacks

                // Play attack sound
                SoundManager::GetInstance().Play("dinoBite");
                // Damage the player
                if (CheckCollisionBoxes(GetBoundingBox(), player.blockHitbox) && player.blocking) {
                    // Blocked!
                    if (rand()%2 == 0){
                        SoundManager::GetInstance().Play("swordBlock");
                    } else{
                        SoundManager::GetInstance().Play("swordBlock2");
                    }
      
                } else  {
                    // Player takes damage
                   
                    player.TakeDamage(10);
                }
  
            }

            if (stateTimer >= randomTime) {
                state = CharacterState::RunAway;
                runawayAngleOffset = DEG2RAD * GetRandomValue(-60, 60);
                hasRunawayAngle = true;
                SetAnimation(3, 4, 0.1f); // run away animation
                stateTimer = 0.0f;
                randomDistance = GetRandomValue(1000, 2000);
            }
            break;


        case CharacterState::RunAway: {
            Vector3 awayDir = Vector3Normalize(Vector3Subtract(position, player.position)); // direction away from player

            if (!hasRunawayAngle) {
                runawayAngleOffset = DEG2RAD * GetRandomValue(-50, 50);
                hasRunawayAngle = true;
            }
            float baseAngle = atan2f(position.z - player.position.z, position.x - player.position.x);
            float finalAngle = baseAngle + runawayAngleOffset;

            Vector3 veerDir = { cosf(finalAngle), 0.0f, sinf(finalAngle) };

            Vector3 horizontalMove = { veerDir.x, 0, veerDir.z };

            // Add repulsion
            Vector3 repulsion = ComputeRepulsionForce(enemyPtrs, 50, 500);
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
                state = CharacterState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
                idleThreshold = (float)GetRandomValue(1, 5);
            }
            if (currentTerrainHeight <= 65.0f) {
                state = CharacterState::Chase;
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                hasRunawayAngle = false;
                //break; // exit RunAway logic
            }
            break;
        }

        case CharacterState::Stagger: {
            //do nothing
            stateTimer += deltaTime;
            if (stateTimer >= 0.6f) {
                canBleed = true;
                state = CharacterState::Chase;
                SetAnimation(1, 5, 0.12f);
                stateTimer = 0.0f;
                chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
            }
            break;
        }
        case CharacterState::Death:
            if (!isDead) {
                SetAnimation(4, 5, 0.15f, false);  
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;
            break;
        }
    }






void Character::UpdatePirateAI(float deltaTime, Player& player) {
    if (isLoadingLevel) return;
    float distance = Vector3Distance(position, player.position);
    playerVisible = false;
    Vector2 start = WorldToImageCoords(position);
    Vector2 goal = WorldToImageCoords(player.position);

    bool canSee = HasWorldLineOfSight(position, player.position, 0.0f); //Vision test // no dungeonLOS test makes for better vision.
    

    if (canSee) {
        playerVisible = true;
        timeSinceLastSeen = 0.0f;
        lastKnownPlayerPos = player.position;
        hasLastKnownPlayerPos = true;
        
    } else {
        timeSinceLastSeen += deltaTime;
        if (timeSinceLastSeen > forgetTime) {
            playerVisible = false;
        }
    }
 
    switch (state){
        case CharacterState::Idle: {
            stateTimer += deltaTime;

            Vector2 start = WorldToImageCoords(position);

            // Transition to chase if player detected
            if (distance < 4000.0f && stateTimer > 1.0f && (playerVisible)) {
                AlertNearbySkeletons(position, 3000.0f);
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.2f);
                stateTimer = 0.0f;

                Vector2 goal = WorldToImageCoords(player.position);
                std::vector<Vector2> tilePath = SmoothPath(FindPath(start, goal), dungeonImg);

                currentWorldPath.clear(); //construct the path the frame before chasing
                for (const Vector2& tile : tilePath) {
                    Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                    worldPos.y += 80.0f;
                    currentWorldPath.push_back(worldPos);
                }
            }
            // Wander if idle too long
            else if (stateTimer > 10.0f) {
                if (TrySetRandomPatrolPath(start, this, currentWorldPath)) {
                    state = CharacterState::Patrol;
                    SetAnimation(1, 4, 0.2f); // walk anim
                }
            }

            break;
        }


        
        case CharacterState::Chase:
            stateTimer += deltaTime;
            pathCooldownTimer -= deltaTime;

            if (distance < 800.0f && canSee) { 
                state = CharacterState::Attack;
                SetAnimation(2, 4, 0.2f);
                stateTimer = 0.0f;
                attackCooldown = 0.0f;

            } 

            else if (!canSee && hasLastKnownPlayerPos) {
                // Plan path to last known position if needed
                if (pathCooldownTimer <= 0.0f) {
                    Vector2 start = WorldToImageCoords(position);
                    Vector2 goal = WorldToImageCoords(lastKnownPlayerPos);
                    std::vector<Vector2> tilePath = SmoothPath(FindPath(start, goal), dungeonImg);

                    currentWorldPath.clear();
                    for (const Vector2& tile : tilePath) {
                        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                        worldPos.y += 80.0f;
                        currentWorldPath.push_back(worldPos);
                    }

                    pathCooldownTimer = 0.4f;
                }
            }
            else if (distance > 4000.0f) {
                state = CharacterState::Idle;
                hasLastKnownPlayerPos = false;
            }
            else {
                Vector2 currentPlayerTile = WorldToImageCoords(player.position);
                if (((int)currentPlayerTile.x != (int)lastPlayerTile.x || (int)currentPlayerTile.y != (int)lastPlayerTile.y)
                    && pathCooldownTimer <= 0.0f) {

                    lastPlayerTile = currentPlayerTile; //save the last player tile so we aren't recalculating if the player hasn't moved. 
                    pathCooldownTimer = 0.4f;

                    Vector2 start = WorldToImageCoords(position);
                    
                    std::vector<Vector2> tilePath = SmoothPath(FindPath(start, currentPlayerTile), dungeonImg);
                    currentWorldPath.clear();
                    for (const Vector2& tile : tilePath) {
                        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                        worldPos.y += 80.0f; //move up to match character height.
                        currentWorldPath.push_back(worldPos);
                    }
                }

                // Move along current path
                if (!currentWorldPath.empty() && state != CharacterState::Stagger) {
                    Vector3 targetPos = currentWorldPath[0];
                    Vector3 dir = Vector3Normalize(Vector3Subtract(targetPos, position));
                    Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);

                                    // Add repulsion from other pirates
                    Vector3 repulsion = ComputeRepulsionForce(enemyPtrs, 50, 500);
                    Vector3 moveWithRepulsion = Vector3Add(move, Vector3Scale(repulsion, deltaTime));
                    position = Vector3Add(position, moveWithRepulsion);
                    rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                    position.y = targetPos.y;

                    if (Vector3Distance(position, targetPos) < 100.0f) { 
                        currentWorldPath.erase(currentWorldPath.begin()); //erase point on arrival, your always chasing the first point on the list. 
                        
                    }

                    if (currentWorldPath.empty() && !canSee) {
                        hasLastKnownPlayerPos = false;
                        state = CharacterState::Idle;
                        SetAnimation(0, 1, 1.0f);
                    }                   
                }
            }
            break;



        case CharacterState::Attack: { //Pirate attack with gun
        
            stateTimer += deltaTime;
            if (stateTimer == 0.0f) {
                SetAnimation(2, 4, 0.2f); // only when entering attack, shoot anim is all the same frame. Try animating it again.
                
            }
            
            Vector2 myTile = WorldToImageCoords(position);
            Character* occupier = GetTileOccupier(myTile.x, myTile.y, enemyPtrs, this);
            //pirates won't occupy the same tile while shooting. 
            if (occupier && occupier != this) {
                // Only the one with the "greater" pointer backs off
                if (this > occupier) {
                    state = CharacterState::Reposition;
                    SetAnimation(1, 4, 0.2f);
                    stateTimer = 0.0f;
                    break;
                } else {
                    // Let the other one reposition — wait
                    break;
                }
            }

            if (distance > 800.0f) { 
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.2f);
                stateTimer = 0.0;
            }
            attackCooldown -= deltaTime;
            if (distance < 800 && distance > 350){
                
                if (canSee && attackCooldown <= 0.0f && currentFrame == 1 && !hasFired) {
                    FireBullet(position, player.position, 1200.0f, 3.0f, true);
                    hasFired = true;
                    attackCooldown = 1.5f;
                    //SoundManager::GetInstance().Play("shotgun");
                    SoundManager::GetInstance().PlaySoundAtPosition("musket", position, player.position, 1.0, 2000);
                }

            }else if (distance < 350){
                state = CharacterState::MeleeAttack;
                SetAnimation(3, 5, 0.12);
                stateTimer = 0;
            }

            // Wait for next attack opportunity
            if (hasFired && stateTimer > 1.5f) {
                hasFired = false;
                attackCooldown = 3.5f;
                currentFrame = 0;
                stateTimer = 0;

                if (TrySetRandomPatrolPath(start, this, currentWorldPath) && canSee) { //shoot then move to a random tile and shoot again.
                    state = CharacterState::Patrol; 
                    SetAnimation(1, 4, 0.2f); // walk anim
                }else{
                    state = CharacterState::Attack;
                    //SetAnimation(1, 4, 0.2f);
                    stateTimer = 0;
                    currentFrame = 0;
                }
               
            }

            break;
        }

        case CharacterState::MeleeAttack: {
            stateTimer += deltaTime;

            // Wait until animation is done to apply damage
            if (stateTimer >= 0.6f && !hasFired) { // 5 frames * 0.12s = 0.6s
                hasFired = true; //reusing hasfired for sword attack. I think this is ok?

                if (distance < 300.0f && playerVisible) {
                    if (CheckCollisionBoxes(GetBoundingBox(), player.blockHitbox) && player.blocking) {
                        // Blocked!
                        if (rand() % 2 == 0) {
                            SoundManager::GetInstance().Play("swordBlock");
                        } else {
                            SoundManager::GetInstance().Play("swordBlock2");
                        }
                    } else {
                        // Direct hit
                        player.TakeDamage(10);
                    }
                }
            }

            // Exit state after full animation plays
            if (stateTimer >= 1.5f) {
                if (distance > 350.0f) {
                    state = CharacterState::Patrol; //we never set the path for this patrol. probably just switches straight to chase. 
                    currentFrame = 0;
                    SetAnimation(1, 5, 0.12f); // walk anim
                } else {
                    
                    // Maybe prepare another melee attack?
                    state = CharacterState::MeleeAttack; // or re-enter melee logic
                    currentFrame = 0;
                    SetAnimation(3, 5, 0.12f); // idle anim
                }
                hasFired= false;
                stateTimer = 0.0f;
            }

            break;
        }


        case CharacterState::Reposition: {
            //skeletons and pirates when close surround the player. Instead of all standing on the same tile. 
            stateTimer += deltaTime;

            Vector2 playerTile = WorldToImageCoords(player.position);
            Vector3 target = position; // fallback

            // Get player's facing direction
            float playerYaw = player.rotation.y;
            Vector3 forward = Vector3Normalize({ sinf(DEG2RAD * playerYaw), 0.0f, cosf(DEG2RAD * playerYaw) });
            Vector3 right = Vector3Normalize(Vector3CrossProduct({ 0, 1, 0 }, forward));

            // Generate 4 offsets relative to player facing
            Vector2 relativeOffsets[4];
            relativeOffsets[0] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(forward, tileSize))) - playerTile;         // front
            relativeOffsets[1] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(right, tileSize))) - playerTile;           // right
            relativeOffsets[2] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(Vector3Negate(right), tileSize))) - playerTile; // left
            relativeOffsets[3] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(Vector3Negate(forward), tileSize))) - playerTile; // back

            bool foundSpot = false;

            for (int i = 0; i < 4; ++i) {
                int tx = (int)playerTile.x + (int)roundf(relativeOffsets[i].x);
                int ty = (int)playerTile.y + (int)roundf(relativeOffsets[i].y);

                if (tx < 0 || ty < 0 || tx >= dungeonWidth || ty >= dungeonHeight) continue;
                if (!IsWalkable(tx, ty, dungeonImg)) continue;
                if (IsTileOccupied(tx, ty, enemyPtrs, this)) continue;

                target = GetDungeonWorldPos(tx, ty, tileSize, dungeonPlayerHeight);
                target.y += 80.0f;
                foundSpot = true;
                break;
            }

            if (foundSpot) {
                foundSpot = false;
                Vector3 dir = Vector3Normalize(Vector3Subtract(target, position));
                Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                position = Vector3Add(position, move);

                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = target.y;

                float dist = Vector3Distance(position, target);

                if (dist < 350.0f && stateTimer > 2.0f) {
                    state = CharacterState::MeleeAttack;
                    SetAnimation(2, 5, 0.2f);
                    stateTimer = 0.0f;
                } else if (dist > 350.0f && stateTimer > 2.0f) {
                    state = CharacterState::Chase;
                    SetAnimation(1, 4, 0.2f);
                    stateTimer = 0.0f;
                }
            }

            break;
        }



        case CharacterState::Patrol: { //Pirate Patrol after every shot. 
            stateTimer += deltaTime;
            //ignore player while patroling to new tile. 

            if (!currentWorldPath.empty()) {
                Vector3 targetPos = currentWorldPath[0];
                Vector3 dir = Vector3Normalize(Vector3Subtract(targetPos, position));
                Vector3 move = Vector3Scale(dir, 500 * deltaTime); // slower than chase //maybe make it even slower, pirates are hard to hit. 
                position = Vector3Add(position, move);
                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = targetPos.y;

                if (Vector3Distance(position, targetPos) < 100.0f) {
                    currentWorldPath.erase(currentWorldPath.begin());
                }
            }
            else {
                state = CharacterState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
            }

            break;
        }


        case CharacterState::Stagger: {
            stateTimer += deltaTime;
            //do nothing
    
           
            if (stateTimer >= 1.0f) {
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.25f);
                stateTimer = 0.0f;
                
            }
            break;
        }

        case CharacterState::Death:
            if (!isDead) {
                SetAnimation(4, 3, 0.5f, false);  
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;
            break;
        }

}

void Character::UpdateSkeletonAI(float deltaTime, Player& player) {

    
    float distance = Vector3Distance(position, player.position);
    playerVisible = false;
    Vector2 start = WorldToImageCoords(position);
    Vector2 goal = WorldToImageCoords(player.position);
    //changed vision to soley rely on world LOS. More forgiving 
    bool canSee = HasWorldLineOfSight(position, player.position, 0.0f); //Vision test //(LineOfSightRaycast(start, goal, dungeonImg, 100, 0.0f)

    if (canSee) {
        playerVisible = true;
        timeSinceLastSeen = 0.0f;
    } else {
        timeSinceLastSeen += deltaTime;
        if (timeSinceLastSeen > forgetTime) {
            playerVisible = false;
        }
    }

    // float distSqr = Vector3DistanceSqr(position, player.position); //Hearing test
    // if (distSqr <= hearingRadius * hearingRadius) {
    //     heardPlayer = true;
    // }

    
 
    switch (state){
        case CharacterState::Idle: {
            stateTimer += deltaTime;
 
            Vector2 start = WorldToImageCoords(position);

            // Transition to chase if player detected
            if (distance < 4000.0f && stateTimer > 1.0f && (playerVisible || heardPlayer)) {
                AlertNearbySkeletons(position, 3000.0f);
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.2f);
                stateTimer = 0.0f;

                Vector2 goal = WorldToImageCoords(player.position);
                std::vector<Vector2> tilePath = SmoothPath(FindPath(start, goal), dungeonImg);

                currentWorldPath.clear();
                for (const Vector2& tile : tilePath) {
                    Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                    worldPos.y += 80.0f;
                    currentWorldPath.push_back(worldPos);
                }
            }

            // Wander if idle too long
            else if (stateTimer > 10.0f) {
                Vector2 randomTile = GetRandomReachableTile(start, this);

                if (randomTile.x != -1) {
                    std::vector<Vector2> tilePath = FindPath(start, randomTile);
                    currentWorldPath.clear();

                    for (const Vector2& tile : tilePath) {
                        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                        worldPos.y += 80.0f;
                        currentWorldPath.push_back(worldPos);
                    }

                    if (!currentWorldPath.empty()) {
                        state = CharacterState::Patrol;
                        SetAnimation(1, 4, 0.2f); // walk anim
                    }
                }
            }

            break;
        }

        
        case CharacterState::Chase:
            stateTimer += deltaTime;
            pathCooldownTimer -= deltaTime;

            if (distance < 300.0f) {
                state = CharacterState::Attack;
                SetAnimation(2, 4, 0.2f);
                stateTimer = 0.0f;
                attackCooldown = 0.0f;

            } else if (distance > 4000.0f) {
                state = CharacterState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;

            } else {
                Vector2 currentPlayerTile = WorldToImageCoords(player.position);
                if (((int)currentPlayerTile.x != (int)lastPlayerTile.x || (int)currentPlayerTile.y != (int)lastPlayerTile.y)
                    && pathCooldownTimer <= 0.0f) {

                    lastPlayerTile = currentPlayerTile; //save the last player tile so we aren't recalculating if the player hasn't moved. 
                    pathCooldownTimer = 0.4f;

                    Vector2 start = WorldToImageCoords(position);
                
                    std::vector<Vector2> tilePath = SmoothPath(FindPath(start, currentPlayerTile), dungeonImg);
                    currentWorldPath.clear();
                    for (const Vector2& tile : tilePath) {
                        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
                        worldPos.y += 80.0f; //move up to match character height.
                        currentWorldPath.push_back(worldPos);
                    }

                    for (Vector2 step : tilePath) {
                        TraceLog(LOG_INFO, "Path step: (%d, %d)", (int)step.x, (int)step.y);
                    }
                }

                // 🧭 Move along current path
                if (!currentWorldPath.empty() && state != CharacterState::Stagger) {
                    Vector3 targetPos = currentWorldPath[0];
                    Vector3 dir = Vector3Normalize(Vector3Subtract(targetPos, position));
                    Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                    position = Vector3Add(position, move);
                    rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                    position.y = targetPos.y;

                    if (Vector3Distance(position, targetPos) < 100.0f) { 
                        currentWorldPath.erase(currentWorldPath.begin()); //erase point on arrival, your always chasing the first point on the list. 
                    }
                }
            }
            break;



        case CharacterState::Attack: {
            //dont stand on the same tile as another skele when attacking
            Vector2 myTile = WorldToImageCoords(position);
            Character* occupier = GetTileOccupier(myTile.x, myTile.y, enemyPtrs, this);

            if (occupier && occupier != this) {
                // Only the one with the "greater" pointer backs off
                if (this > occupier) {
                    state = CharacterState::Reposition;
                    SetAnimation(1, 4, 0.2f);
                    stateTimer = 0.0f;
                    break;
                } else {
                    // Let the other one reposition — wait
                    break;
                }
            }

            if (distance > 350.0f) { 
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.2f);
                stateTimer = 0.0;
            }



            attackCooldown -= deltaTime;
            if (attackCooldown <= 0.0f && currentFrame == 1 && canSee) { // make sure you can see what your attacking. 
                attackCooldown = 1.0f; // 1 second cooldown for 1 second of animation. 

                // Play attack sound
                if (type == CharacterType::Skeleton) SoundManager::GetInstance().Play("swipe3");
                if (type == CharacterType::Spider){
                    if (rand() % 2 == 0){
                        SoundManager::GetInstance().Play("spiderBite1");
                    }else{
                        SoundManager::GetInstance().Play("spiderBite2");
                    }
                    
                } 
                

                // Damage the player
                if (CheckCollisionBoxes(GetBoundingBox(), player.blockHitbox) && player.blocking) {
                    // Blocked!
                    if (rand()%2 == 0){
                        SoundManager::GetInstance().Play("swordBlock");
                    } else{
                        SoundManager::GetInstance().Play("swordBlock2");
                    }
      
                } else  {
                    // Player takes damage
                    //player.TakeDamage(skele.attackPower);
                    player.TakeDamage(10);
                }
                
            }
            break;
        }
        case CharacterState::Reposition: {
            //surround the player
            stateTimer += deltaTime;

            Vector2 playerTile = WorldToImageCoords(player.position);
            Vector3 target = position; // fallback

            // Get player's facing direction
            float playerYaw = player.rotation.y;
            Vector3 forward = Vector3Normalize({ sinf(DEG2RAD * playerYaw), 0.0f, cosf(DEG2RAD * playerYaw) });
            Vector3 right = Vector3Normalize(Vector3CrossProduct({ 0, 1, 0 }, forward));

            // Generate 4 offsets relative to player facing
            Vector2 relativeOffsets[4];
            relativeOffsets[0] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(forward, tileSize))) - playerTile;         // front
            relativeOffsets[1] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(right, tileSize))) - playerTile;           // right
            relativeOffsets[2] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(Vector3Negate(right), tileSize))) - playerTile; // left
            relativeOffsets[3] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(Vector3Negate(forward), tileSize))) - playerTile; // back

            bool foundSpot = false;

            for (int i = 0; i < 4; ++i) {
                int tx = (int)playerTile.x + (int)roundf(relativeOffsets[i].x);
                int ty = (int)playerTile.y + (int)roundf(relativeOffsets[i].y);

                if (tx < 0 || ty < 0 || tx >= dungeonWidth || ty >= dungeonHeight) continue;
                if (!IsWalkable(tx, ty, dungeonImg)) continue;
                if (IsTileOccupied(tx, ty, enemyPtrs, this)) continue;

                target = GetDungeonWorldPos(tx, ty, tileSize, dungeonPlayerHeight);
                target.y += 80.0f;
                foundSpot = true;
                break;
            }

            if (foundSpot) {
                Vector3 dir = Vector3Normalize(Vector3Subtract(target, position));
                Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                position = Vector3Add(position, move);

                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = target.y;

                float dist = Vector3Distance(position, target);

                if (dist < 350.0f && stateTimer > 1.0f) {
                    state = CharacterState::Attack;
                    SetAnimation(2, 5, 0.2f);
                    stateTimer = 0.0f;
                } else if (dist > 350.0f && stateTimer > 1.0f) {
                    state = CharacterState::Chase;
                    SetAnimation(1, 4, 0.2f);
                    stateTimer = 0.0f;
                }
            }

            break;
        }



        case CharacterState::Patrol: {
            stateTimer += deltaTime;

            if (distance < 4000.0f && (playerVisible || heardPlayer)){
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.2f);
                AlertNearbySkeletons(position, 3000.0f);
                stateTimer = 0.0f;
            }

            if (!currentWorldPath.empty()) {
                Vector3 targetPos = currentWorldPath[0];
                Vector3 dir = Vector3Normalize(Vector3Subtract(targetPos, position));
                Vector3 move = Vector3Scale(dir, 500 * deltaTime); // slower than chase
                position = Vector3Add(position, move);
                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = targetPos.y;

                if (Vector3Distance(position, targetPos) < 100.0f) {
                    currentWorldPath.erase(currentWorldPath.begin());
                }
            }
            else {
                state = CharacterState::Idle;
                SetAnimation(0, 1, 1.0f);
                stateTimer = 0.0f;
            }

            break;
        }


        case CharacterState::Stagger: {
            stateTimer += deltaTime;
            //do nothing
    
            //currentWorldPath.clear(); //loose the path on stagger
            if (stateTimer >= 1.0f) {
                canBleed = true; //for spiders
                state = CharacterState::Chase;
                SetAnimation(1, 4, 0.25f);
                stateTimer = 0.0f;
                
            }
            break;
        }

        case CharacterState::Death:
            if (!isDead) {
                SetAnimation(4, 3, 0.5f, false);  
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;
            break;
        }
       
}



BoundingBox Character::GetBoundingBox() const {
    float halfWidth = (frameWidth * scale * 0.4f) / 2.0f;  // Only 40 percent the width of the frame
    float halfHeight = (frameHeight * scale) / 2.0f;

    return {
        { position.x - halfWidth, position.y - halfHeight, position.z - halfWidth },
        { position.x + halfWidth, position.y + halfHeight, position.z + halfWidth }
    };
}

void Character::playRaptorSounds(){

    int rn = GetRandomValue(1, 3);
    //std::cout << "playing sound";
    switch (rn)
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

    if (currentHealth <= 0) { //die

        
        hitTimer = 0.5;
        currentHealth = 0;
        isDead = true;
        deathTimer = 0.0f;
        state = CharacterState::Death;

        if (type != CharacterType::Skeleton){
            bloodEmitter.EmitBlood(position, 20);
        }
        if (type == CharacterType::Raptor) SetAnimation(4, 5, 0.12f, false);
        if (type == CharacterType::Skeleton) SetAnimation(4, 3, 0.5f, false); //less frames for skele death.
        if (type == CharacterType::Pirate) SetAnimation(4, 2, 1, false);
        if (type == CharacterType::Spider) SetAnimation(4, 3, 0.5f, false);
        
        if (type != CharacterType::Spider)  SoundManager::GetInstance().Play("dinoDeath");
        if (type == CharacterType::Skeleton) SoundManager::GetInstance().Play("bones");
        if (type == CharacterType::Spider) SoundManager::GetInstance().Play("spiderDeath");
     
    } else {
        hitTimer = 0.5f; //tint red
        state = CharacterState::Stagger;
        if (type != CharacterType::Skeleton && canBleed){
            canBleed = false;
            bloodEmitter.EmitBlood(position, 20);
        }
        SetAnimation(4, 1, 1.0f); // Use first frame of death anim for 1 second. for all enemies
        currentFrame = 0;         // Always start at first frame
        stateTimer = 0.0f;
        AlertNearbySkeletons(position, 3000.0f);

        if (type == CharacterType::Pirate){
            SoundManager::GetInstance().Play("phit1");
        }else if (type == CharacterType::Spider){
            SoundManager::GetInstance().Play("spiderDeath");
        }else{
            SoundManager::GetInstance().Play("dinoHit"); //raptor and skeletons
        }
        
    }
}

void Character::AlertNearbySkeletons(Vector3 alertOrigin, float radius) {
    Vector2 originTile = WorldToImageCoords(alertOrigin);

    for (Character& other : enemies) {
        if (&other == this) continue; // Don't alert yourself
        if (other.isDead || other.state == CharacterState::Chase) continue;

        float distSqr = Vector3DistanceSqr(alertOrigin, other.position);
        if (distSqr > radius * radius) continue;

        Vector2 targetTile = WorldToImageCoords(other.position);
        if (!LineOfSightRaycast(originTile, targetTile, dungeonImg, 60, 0.0f)) continue;

        // Passed all checks → alert the skeleton
        other.state = CharacterState::Chase;
        other.SetAnimation(1, 4, 0.2f);
        other.stateTimer = 0.0f;
        other.playerVisible = true;
    }
}





Vector3 Character::ComputeRepulsionForce(const std::vector<Character*>& allRaptors, float repulsionRadius, float repulsionStrength) {
    Vector3 repulsion = { 0 };
    //prevent raptors overlapping 
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


void Character::eraseCharacters() {
    // Remove dead enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Character& e) {
            return e.isDead && e.deathTimer > 5.0f;
        }),
        enemies.end());

    // Rebuild enemyPtrs
    enemyPtrs.clear();
    for (auto& e : enemies) {
        enemyPtrs.push_back(&e);
    }
}



void Character::Update(float deltaTime, Player& player,const  Image& heightmap, Vector3 terrainScale ) {
    if (isLoadingLevel) return;
    bloodEmitter.UpdateBlood(deltaTime);
    animationTimer += deltaTime;
    stateTimer += deltaTime;
    previousPosition = position;
    float groundY = GetHeightAtWorldPosition(position, heightmap, terrainScale); //get groundY from heightmap
    if (isDungeon) groundY = dungeonPlayerHeight;

    if (hitTimer > 0){
        hitTimer -= deltaTime;
    }else{
        hitTimer = 0;
    }
    // Gravity
    float gravity = 980.0f; //we need gravity for outside maps so characters stick to heightmap.
    if (isDungeon) gravity = 0.0f; //no gravity in dungeons. floor is fixed height. 
    static float verticalVelocity = 0.0f;

    float spriteHeight = frameHeight * scale;

    if (position.y > groundY + spriteHeight / 2.0f) {
        verticalVelocity -= gravity * deltaTime;
        position.y += verticalVelocity * deltaTime;
    } else {
        verticalVelocity = 0.0f;
        position.y = groundY + spriteHeight / 2.0f;
    }
    
   
    UpdateAI(deltaTime,player, heightmap, terrainScale);



    // Advance animation frame
    if (animationTimer >= animationSpeed) {
        animationTimer = 0;


        if (state == CharacterState::Death) {
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
    //never called. we draw billboards in transparentDraw
    Rectangle sourceRec = {
        (float)(currentFrame * frameWidth),
        (float)(rowIndex * frameHeight),
        (float)frameWidth,
        (float)frameHeight
    };

    // Calculate a slight camera-facing offset to reduce z-fighting
    Vector3 camDir = Vector3Normalize(Vector3Subtract(camera.position, position));
    Vector3 offsetPos = Vector3Add(position, Vector3Scale(camDir, 10.0f));
    if (type == CharacterType::Skeleton) scale = 0.8; //resize skele before drawing
    Vector2 size = { frameWidth * scale, frameHeight * scale };

    Color redTint = (hitTimer > 0.0f) ? (Color){255, 50, 50, 255} : WHITE;
    rlDisableDepthMask();
    if (isDead && type == CharacterType::Pirate) offsetPos.y -= 25; 
    if (isDead && type == CharacterType::Spider) offsetPos.y -= 25; 
    //DrawBoundingBox(GetBoundingBox(), RED); //debug visible bounding boxes
    DrawBillboardRec(camera, *texture, sourceRec, offsetPos, size, redTint);

    rlEnableDepthMask();
}


void Character::SetAnimation(int row, int frames, float speed, bool loop) {
    rowIndex = row;
    maxFrames = frames;
    animationSpeed = speed;
    currentFrame = 0;
    animationTimer = 0;
    animationLoop = loop;
}
