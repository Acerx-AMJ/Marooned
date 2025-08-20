#include "character.h"
#include "raylib.h"
#include "raymath.h"
#include "dungeonGeneration.h"
#include "world.h"
#include "pathfinding.h"
#include "sound_manager.h"
#include "resourceManager.h"
#include "utilities.h"

void Character::UpdateAI(float deltaTime, Player& player) {
    switch (type) {
        case CharacterType::Raptor:
            UpdateRaptorAI(deltaTime, player);
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

        case CharacterType::Ghost:
            UpdateSkeletonAI(deltaTime, player);
            break;
    }
}

void Character::UpdatePlayerVisibility(const Vector3& playerPos, float dt, float epsilon) {
    canSee = HasWorldLineOfSight(position, playerPos, epsilon);

    if (canSee) {
        lastKnownPlayerPos = playerPos;
        playerVisible = true;
        timeSinceLastSeen = 0.0f;
    } else {
        timeSinceLastSeen += dt;
        if (timeSinceLastSeen > forgetTime) {
            playerVisible = false;
        }
    }
}



void Character::UpdateSkeletonAI(float deltaTime, Player& player) {
    float distance = Vector3Distance(position, player.position);

    Vector2 start = WorldToImageCoords(position);
    Vector2 goal = WorldToImageCoords(player.position);
    //changed vision to soley rely on world LOS. More forgiving 
    playerVisible = false;
    UpdatePlayerVisibility(player.position, deltaTime, 0.0f);

 
    switch (state){
        case CharacterState::Idle: {
            stateTimer += deltaTime;
 
            Vector2 start = WorldToImageCoords(position);

            // Transition to chase if player detected
            if (distance < 4000.0f && stateTimer > 1.0f && playerVisible) {
                AlertNearbySkeletons(position, 3000.0f);
                ChangeState(CharacterState::Chase);
                currentWorldPath.clear();

                SetPath(start);
            }

            // Wander if idle too long
            else if (stateTimer > 10.0f) {
                Vector2 randomTile = GetRandomReachableTile(start, this);

                if (IsWalkable(randomTile.x, randomTile.y, dungeonImg)) {
                    if (TrySetRandomPatrolPath(start, this, currentWorldPath)) {
                        state = CharacterState::Patrol;
                        SetAnimation(1, 4, 0.2f); // walk anim
                        if (type == CharacterType::Ghost) SetAnimation(0, 7, 0.2, true);
                    }
                }
            }

            break;
        }

        
        case CharacterState::Chase: {
            stateTimer += deltaTime;
            pathCooldownTimer = std::max(0.0f, pathCooldownTimer - deltaTime);

            if (distance < 300.0f && canSee) {
                ChangeState(CharacterState::Attack);

            }
            else if (distance > 4000.0f) {
                ChangeState(CharacterState::Idle);

            }
            else {
                const Vector2 curTile = WorldToImageCoords(player.position);
                if (((int)curTile.x != (int)lastPlayerTile.x || (int)curTile.y != (int)lastPlayerTile.y)
                    && pathCooldownTimer <= 0.0f)
                {
                    lastPlayerTile = curTile;
                    pathCooldownTimer = 0.4f; // don’t spam BFS
                    const Vector2 start = WorldToImageCoords(position);
                    SetPath(start); 
                }

                // Move along current path
                MoveAlongPath(currentWorldPath, position, rotationY, skeleSpeed, deltaTime, 100.0f);
            }
        } break;



        case CharacterState::Attack: {
            //dont stand on the same tile as another skele when attacking
            Vector2 myTile = WorldToImageCoords(position);
            Character* occupier = GetTileOccupier(myTile.x, myTile.y, enemyPtrs, this);

            if (occupier && occupier != this) {
                // Only the one with the "greater" pointer backs off
                if (this > occupier) {
                    ChangeState(CharacterState::Reposition);
                    break;
                } else {
                    // Let the other one reposition — wait
                    break;
                }
            }

            if (distance > 350.0f) { 
                ChangeState(CharacterState::Chase);

            }

            attackCooldown -= deltaTime;
            if (attackCooldown <= 0.0f && currentFrame == 1 && playerVisible) { // make sure you can see what your attacking. 
                attackCooldown = 0.8f; // 0.2 * 4 frames on animation for skele attack. 

                // Play attack sound
                if (type == CharacterType::Skeleton) SoundManager::GetInstance().Play("swipe3");
                if (type == CharacterType::Spider){
                    if (rand() % 2 == 0){
                        SoundManager::GetInstance().Play("spiderBite1");
                    }else{
                        SoundManager::GetInstance().Play("spiderBite2");
                    }
                    
                } 
                
                    // Blocked!
                if (CheckCollisionBoxes(GetBoundingBox(), player.blockHitbox) && player.blocking) {

                    if (rand()%2 == 0){
                        SoundManager::GetInstance().Play("swordBlock");
                    } else{
                        SoundManager::GetInstance().Play("swordBlock2");
                    }
      
                } else  {
                    // Player takes damage
                    
                    player.TakeDamage(10);

                    if (type == CharacterType::Ghost){
                        Vector3 mid = Vector3Lerp(position, player.position, 0.5f); //in between ghost and player
                        decals.emplace_back(mid, DecalType::MagicAttack, R.GetTexture("magicAttackSheet"), 8, 1.0f, 0.1f, 60.0f);
                        // siphon heal
                        int healOnHit = 20; 
                        currentHealth = std::min(maxHealth, currentHealth + healOnHit);
                        player.TakeDamage(10);
                    }

                }
                
            }
            break;
        }
        case CharacterState::Reposition: {
            //surround the player
            stateTimer += deltaTime;

            Vector2 playerTile = WorldToImageCoords(player.position);
            Vector3 target = position; // fallback

            bool foundSpot = FindRepositionTarget(player, position, target);

            if (foundSpot) {
                Vector3 dir = Vector3Normalize(Vector3Subtract(target, position));
                Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                position = Vector3Add(position, move);

                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = target.y;

                float dist = Vector3Distance(position, target);

                if (dist < 350.0f && stateTimer > 1.0f) {
                    ChangeState(CharacterState::Attack);
                } else if (dist > 350.0f && stateTimer > 1.0f) {
                    ChangeState(CharacterState::Chase);
                }
            }

            break;
        }



        case CharacterState::Patrol: {
            stateTimer += deltaTime;

            if (distance < 4000.0f && playerVisible){
                ChangeState(CharacterState::Chase);
                AlertNearbySkeletons(position, 3000.0f);

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
                ChangeState(CharacterState::Idle);
            }

            break;
        }

        case CharacterState::Freeze: {
            stateTimer += deltaTime;
            //do nothing

            if (stateTimer > 5.0f){
                ChangeState(CharacterState::Idle);
            }
            break;

        }


        case CharacterState::Stagger: {
            stateTimer += deltaTime;
            //do nothing

            if (stateTimer >= 1.0f) {
                canBleed = true; //for spiders
                ChangeState(CharacterState::Chase);
                
            }
            break;
        }

        case CharacterState::Death:
            if (!isDead) {
                SetAnimation(4, 3, 0.5f, false); 
                if (type == CharacterType::Ghost) SetAnimation(1, 7, 0.2); 
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;
            break;
        }
       
}

// Raptor = overworld, no grid pathing.
// Skeleton only (structure + thresholds). Fill TODOs as you add steering.

void Character::UpdateRaptorAI(float dt, Player& player)
{
    // --- Perception & timers ---
    stateTimer     += dt;
    attackCooldown  = std::max(0.0f, attackCooldown - dt);

    const float distance = Vector3Distance(position, player.position);
   

    // --- Simple deadbands (tweak per type) ---
    const float STALK_ENTER   = 2000.0f;  // engage if closer than this
    const float STALK_EXIT    = 2400.0f;  // drop back to idle if beyond and no memory
    const float ATTACK_ENTER  = 200.0f;   // start attack if closer than this (+ LOS)
    const float ATTACK_EXIT   = 300.0f;   // leave attack if beyond this or LOS lost
    const float FLEE_ENTER    = 100.0f;   // too close -> run away
    const float FLEE_EXIT     = 1000.0f;   // far enough -> stop fleeing
    const float VISION_ENTER = 4000.0f;

    playerVisible = distance < VISION_ENTER;

    // --- Placeholder for steering output (fill later) ---
    Vector3 desiredVel = {0,0,0};   // compute with Seek/Arrive/Orbit/Flee later

    switch (state)
    {

        case CharacterState::Idle:
        {
            const float IDLE_TO_PATROL_TIME = 10.0f; // tweak
            if (stateTimer >= IDLE_TO_PATROL_TIME) {
                // seed a patrol target around the current position
                patrolTarget = RandomPointOnRingXZ(position, /*minR*/800.0f, /*maxR*/2200.0f);

                hasPatrolTarget  = true;
                ChangeState(CharacterState::Patrol);
                break;
            }

            if (distance < STALK_ENTER && playerVisible) {
                ChangeState(CharacterState::Chase);
                chaseDuration = GetRandomValue(5, 8);
                break;
            }
        } break;

        case CharacterState::Patrol:
        {
            // If we somehow lost the target, pick a new one quickly
            if (!hasPatrolTarget) {
                patrolTarget    = RandomPointOnRingXZ(position, 800.0f, 2200.0f);
                
                hasPatrolTarget = true;
            }

            // Move straight toward target (ease-in near the point)
            const float PATROL_SPEED      = raptorSpeed * 0.6f; // slower than chase
            const float PATROL_SLOW_RAD   = 400.0f;
            const float ARRIVE_EPS_XZ     = 150.0f;

            Vector3 vel = ArriveXZ(position, patrolTarget, PATROL_SPEED, PATROL_SLOW_RAD);
            position = Vector3Add(position, Vector3Scale(vel, dt));
           

            if (vel.x*vel.x + vel.z*vel.z > 1e-4f) {
                rotationY = RAD2DEG * atan2f(vel.x, vel.z);
            }

            // Arrived? go Idle and reset
            if (DistXZ(position, patrolTarget) <= ARRIVE_EPS_XZ) {
                hasPatrolTarget = false;
                ChangeState(CharacterState::Idle);
                break;
            }

            // If player shows up while patrolling, escalate to Chase
            const float STALK_ENTER = 2000.0f;
            if (playerVisible && Vector3Distance(position, player.position) < STALK_ENTER) {
                hasPatrolTarget = false; // drop current patrol
                ChangeState(CharacterState::Chase);
                chaseDuration = GetRandomValue(5, 8); // 3–7 seconds of chasing
                break;
            }
        } break;

        case CharacterState::Chase:
        {
            if (stateTimer > chaseDuration) { ChangeState(CharacterState::RunAway); break;}
                    // (keep your existing enter/exit checks above or below as you like)
            if (distance < ATTACK_ENTER) { ChangeState(CharacterState::Attack); break; }
            if (distance > VISION_ENTER) { ChangeState(CharacterState::Idle); break; }

            const float MAX_SPEED   = raptorSpeed;  // per-type speed
            const float SLOW_RADIUS = 400.0f;       // ease-in so we don’t overshoot

            // Move straight toward the player (XZ only), easing inside SLOW_RADIUS
            Vector3 vel = ArriveXZ(position, player.position, MAX_SPEED, SLOW_RADIUS);
            position = Vector3Add(position, Vector3Scale(vel, dt));
            //ClampToTerrain(position, /*footOffset*/0.0f);

            if (vel.x*vel.x + vel.z*vel.z > 1e-4f) {
                rotationY = RAD2DEG * atan2f(vel.x, vel.z);
            }

          
        } break;

        case CharacterState::Attack:
        {
            if (distance < FLEE_ENTER) { ChangeState(CharacterState::RunAway); break;}
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

            if (distance > ATTACK_EXIT) {
                ChangeState(CharacterState::Chase);
                chaseDuration = GetRandomValue(5, 8); // 3–7 seconds of chasing
                break;
            }


        } break;

        case CharacterState::RunAway:
        {
            // --- simple knobs ---
            const float MAX_SPEED     = raptorSpeed;   // same as chase or a bit higher
            const float FLEE_MIN_TIME = 0.7f;          // don’t instantly flip back
            const float FLEE_MAX_TIME = 2.0f;          // optional: cap flee bursts
            const float FLEE_EXIT     = 400.0f;        // you already defined this earlier
            const float SEP_CAP       = 200.0f;        // limit separation shove

            // Steering: flee + a touch of separation + tiny wander so it’s not laser-straight
            Vector3 vFlee   = FleeXZ(position, player.position, MAX_SPEED);

            // If you have a raptor list handy; otherwise set to {0,0,0}
            Vector3 vSep    = ComputeRepulsionForce(enemyPtrs, /*radius*/120, /*falloff*/600);
            vSep            = Limit(vSep, SEP_CAP);

            
            Vector3 vWander = WanderXZ(wanderAngle, /*turn*/4.0f, /*speed*/80.0f, dt);

            Vector3 desired = Vector3Add(vFlee, Vector3Add(vSep, vWander));
            desired         = Limit(desired, MAX_SPEED);

            // Integrate + face motion
            position = Vector3Add(position, Vector3Scale(desired, dt));
            if (desired.x*desired.x + desired.z*desired.z > 1e-4f) {
                rotationY = RAD2DEG * atan2f(desired.x, desired.z);
            }

            // Exit conditions: far enough OR time window expired
            if ((distance > FLEE_EXIT && stateTimer >= FLEE_MIN_TIME) || stateTimer >= FLEE_MAX_TIME) {
                ChangeState(CharacterState::Chase);
                break;
            }
        } break;


        case CharacterState::Reposition:
        {
            // (Optional for raptors) quick side-step if crowded
            // TODO: short timer, then back to Chase
            if (stateTimer > 0.5f) {
                ChangeState(CharacterState::Chase);
                break;
            }
        } break;

        case CharacterState::MeleeAttack:
        {
            // (If you separate melee from generic Attack later)
            // TODO: same pattern as Attack with its own timings
        } break;

        case CharacterState::Freeze:
        {
            // TODO: do nothing; exit after freezeDuration or on event
            // if (stateTimer >= freezeDuration) ChangeState(CharacterState::Chase);
        } break;

        case CharacterState::Stagger:
        {

            desiredVel = {0,0,0};
            
            //do nothing
 
            if (stateTimer >= 1.0f) {
                canBleed = true;
                ChangeState(CharacterState::Chase);
                
                chaseDuration = GetRandomValue(4, 8); 
            }
        } break;

        case CharacterState::Death:
        {
            if (!isDead) {
                isDead = true;
                deathTimer = 0.0f;// Start counting
            }

            deathTimer += dt;
        } break;
    }

    // --- Integrate movement (fill once steering is in) ---
    // position = Vector3Add(position, Vector3Scale(desiredVel, dt));
    // ClampToTerrain(position, /*footOffset*/0.0f);
    // if (desiredVel.x*desiredVel.x + desiredVel.z*desiredVel.z > 1e-4f)
    //     rotationY = RAD2DEG * atan2f(desiredVel.x, desiredVel.z);
}


// void Character::UpdateRaptorAI(float deltaTime, Player& player) {
//     if (isLoadingLevel) return;
//     float distance = Vector3Distance(position, player.position);
//     playerVisible = false;

//     if (!isDungeon && distance < 4000){
//         playerVisible = true; //player is always visible to raptors within range. 
//     }

//     switch (state) {
//         case CharacterState::Idle:
//             if (stateTimer > idleThreshold){ //if idle for 10 seconds run to a new spot. 
//                 ChangeState(CharacterState::RunAway);
//                 randomTime = GetRandomValue(5,15);

//                 runawayAngleOffset = DEG2RAD * GetRandomValue(-180, 180);
//                 hasRunawayAngle = true;
//                 //playRaptorSounds(); //make noises while they run around
//             }

//             if (distance < 4000.0f && stateTimer > 1.0f) { 

//                 //if (isDungeon) setPath();
//                 ChangeState(CharacterState::Chase);
//                 chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
//                 hasRunawayAngle = false;

//                 //int number = GetRandomValue(1, 3);
//                 playRaptorSounds();

//             }
//             break;

//         case CharacterState::Chase: {
//             stateTimer += deltaTime;

//             if (distance < 150.0f) {
//                 //Gun is 100 away from player center, don't clip the gun. 
//                 ChangeState(CharacterState::Attack);
//                 randomTime = GetRandomValue(5,15);
//             } else if (distance > 4000.0f) {
//                 ChangeState(CharacterState::Idle);
//                 idleThreshold = (float)GetRandomValue(5,15);
//             } else if (stateTimer >= chaseDuration && !isDungeon) {
//                 // Give up and run away
//                 ChangeState(CharacterState::RunAway);
//                 runawayAngleOffset = DEG2RAD * GetRandomValue(-50, 50); //run in a random direction
//                 randomDistance = GetRandomValue(1000, 2000); //set distance to run 
//                 if (isDungeon) randomDistance = GetRandomValue(500, 1000);
//                 hasRunawayAngle = true;
//             } else {

//                 // Chase logic with repulsion
//                 Vector3 dir = Vector3Normalize(Vector3Subtract(player.position, position));
//                 Vector3 horizontalMove = { dir.x, 0, dir.z };

//                 // Add repulsion from other raptors
//                 Vector3 repulsion = ComputeRepulsionForce(enemyPtrs, 50, 500);
//                 Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

//                 Vector3 proposedPosition = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f)); //one step ahead. 

//                 float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPosition, heightmap, terrainScale); //see if the next step is water. 
//                 float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
//                 //float spriteHeight = frameHeight * scale;
//                 if (isDungeon) proposedTerrainHeight = dungeonPlayerHeight;
//                 if (isDungeon) currentTerrainHeight = dungeonPlayerHeight;
//                 //run away if near water. X Raptor now stops at waters edge, he no longer gets stuck though.
//                 if (currentTerrainHeight <= 65.0f && stateTimer > 1.0f) {
//                     state = CharacterState::RunAway;
//                     SetAnimation(3, 4, 0.1f);
//                     stateTimer = 0.0f;
//                     randomDistance = GetRandomValue(1000, 2000); //set distance to run 
//                     runawayAngleOffset = DEG2RAD * GetRandomValue(-50, 50); //run in a random direction
//                     stateTimer = 0;

//                 } else if (proposedTerrainHeight > 60.0f) {
//                     position = proposedPosition;
//                     rotationY = RAD2DEG * atan2f(dir.x, dir.z);
//                     //dont recalculate height if in a dungeon. dungeon height stays the same. 
//                     if (!isDungeon) position.y = GetHeightAtWorldPosition(position, heightmap, terrainScale) + (frameHeight * scale) / 2.0f; //recalculate height
//                 }

//             }

//             break;
//         }
//         case CharacterState::Attack:
//             if (distance > 200.0f) { //maybe this should be like 160, player would get bit more. 
//                 ChangeState(CharacterState::Chase);
//             }

//             attackCooldown -= deltaTime;
//             if (attackCooldown <= 0.0f) {
//                 attackCooldown = 1.0f; // seconds between attacks

//                 // Play attack sound
//                 SoundManager::GetInstance().Play("dinoBite");
//                 // Damage the player
//                 if (CheckCollisionBoxes(GetBoundingBox(), player.blockHitbox) && player.blocking) {
//                     // Blocked!
//                     if (rand()%2 == 0){
//                         SoundManager::GetInstance().Play("swordBlock");
//                     } else{
//                         SoundManager::GetInstance().Play("swordBlock2");
//                     }
      
//                 } else  {
//                     // Player takes damage
                   
//                     player.TakeDamage(10);
//                 }
  
//             }

//             if (stateTimer >= randomTime) {
//                 runawayAngleOffset = DEG2RAD * GetRandomValue(-60, 60);
//                 hasRunawayAngle = true;
//                 ChangeState(CharacterState::RunAway);
//                 randomDistance = GetRandomValue(1000, 2000);
//             }
//             break;


//         case CharacterState::RunAway: {
//             Vector3 awayDir = Vector3Normalize(Vector3Subtract(position, player.position)); // direction away from player

//             if (!hasRunawayAngle) {
//                 runawayAngleOffset = DEG2RAD * GetRandomValue(-50, 50);
//                 hasRunawayAngle = true;
//             }
//             float baseAngle = atan2f(position.z - player.position.z, position.x - player.position.x);
//             float finalAngle = baseAngle + runawayAngleOffset;

//             Vector3 veerDir = { cosf(finalAngle), 0.0f, sinf(finalAngle) };

//             Vector3 horizontalMove = { veerDir.x, 0, veerDir.z };

//             // Add repulsion
//             Vector3 repulsion = ComputeRepulsionForce(enemyPtrs, 50, 500);
//             Vector3 moveWithRepulsion = Vector3Add(horizontalMove, Vector3Scale(repulsion, deltaTime));

//             Vector3 proposedPos = Vector3Add(position, Vector3Scale(moveWithRepulsion, deltaTime * 700.0f));
//             float currentTerrainHeight = GetHeightAtWorldPosition(position, heightmap, terrainScale);
//             float proposedTerrainHeight = GetHeightAtWorldPosition(proposedPos, heightmap, terrainScale);
//             if (isDungeon) proposedTerrainHeight = dungeonPlayerHeight;
//             if (isDungeon) currentTerrainHeight = dungeonPlayerHeight;
//             // If the current terrain is near water, force runaway state to continue,
//             // but only move if the proposed position is on solid ground.
//             if (proposedTerrainHeight > 60.0f) {
//                 position = proposedPos;
//                 position.y = proposedTerrainHeight + (frameHeight * scale) / 2.0f;
//                 rotationY = RAD2DEG * atan2f(awayDir.x, awayDir.z);
//             }

            
//             if (distance > randomDistance && stateTimer > 1.0f && currentTerrainHeight > 60.0f) {
//                 ChangeState(CharacterState::Idle);
//                 idleThreshold = (float)GetRandomValue(1, 5);
//             }
//             if (currentTerrainHeight <= 65.0f) {
//                 ChangeState(CharacterState::Chase);
//                 hasRunawayAngle = false;
//                 //break; // exit RunAway logic
//             }
//             break;
//         }

//         case CharacterState::Freeze: {
//             stateTimer += deltaTime;
//             //do nothing

//             if (stateTimer > 5.0f){
//                 ChangeState(CharacterState::Idle);
//             }
//             break;

//         }

//         case CharacterState::Stagger: {
//             //do nothing
//             stateTimer += deltaTime;
//             if (stateTimer >= 0.6f) {
//                 canBleed = true;
//                 ChangeState(CharacterState::Chase);
//                 chaseDuration = GetRandomValue(3, 7); // 3–7 seconds of chasing
//             }
//             break;
//         }
//         case CharacterState::Death: {
//             if (!isDead) {
                
//                 isDead = true;
//                 deathTimer = 0.0f;         // Start counting
//             }

//             deathTimer += deltaTime;
//             break;
//         }
//     }
// }

void Character::UpdatePirateAI(float deltaTime, Player& player) {
    if (isLoadingLevel) return;
    constexpr float PIRATE_ATTACK_ENTER = 800.0f; // start attacking when closer than this
    constexpr float PIRATE_ATTACK_EXIT  = 900.0f; // stop attacking when farther than this
    // (EXIT must be > ENTER)

    
    float distance = Vector3Distance(position, player.position);
    float pirateHeight = 160;
    playerVisible = false;
    Vector2 start = WorldToImageCoords(position);
    Vector2 goal = WorldToImageCoords(player.position);

    UpdatePlayerVisibility(player.position, deltaTime, 0.0f);

 
    switch (state){
        case CharacterState::Idle: {
            stateTimer += deltaTime;

            Vector2 start = WorldToImageCoords(position);

            // Transition to chase if player detected
            if (distance < 4000.0f && stateTimer > 1.0f && (playerVisible)) {
                AlertNearbySkeletons(position, 3000.0f);
                ChangeState(CharacterState::Chase);
                SetPath(start);

            }
            // Wander if idle too long
            else if (stateTimer > 10.0f) {
                if (TrySetRandomPatrolPath(start, this, currentWorldPath)) {
                    ChangeState(CharacterState::Patrol);
                }
            }

            break;
        }
        
        case CharacterState::Chase: {
            stateTimer += deltaTime;
            pathCooldownTimer = std::max(0.0f, pathCooldownTimer - deltaTime);

            // 1) Try to attack when close AND we have instant LOS
            if (distance < PIRATE_ATTACK_ENTER && canSee) {
                ChangeState(CharacterState::Attack);
                break;
            }

            // 2) Leash out if too far
            if (distance > 4000.0f) {
                ChangeState(CharacterState::Idle);
                playerVisible = false;         // drop memory when giving up
                currentWorldPath.clear();
                break;
            }

            // 3) Plan path toward current target when cooldown allows
            if (pathCooldownTimer <= 0.0f) {
                if (canSee) {
                    SetPathTo(player.position);
                    pathCooldownTimer = 0.4f;
                } else if (playerVisible) {       // still within memory window
                    SetPathTo(lastKnownPlayerPos);
                    pathCooldownTimer = 0.4f;
                }
            }


            // 4) Advance along path (with repulsion)
            if (!currentWorldPath.empty() && state != CharacterState::Stagger) {
                Vector3 repel = ComputeRepulsionForce(enemyPtrs, 50, 500); // your existing call
                MoveAlongPath(currentWorldPath, position, rotationY, skeleSpeed, deltaTime, 100.0f, repel);

                // Reached the end but still no LOS? stop chasing
                if (currentWorldPath.empty() && !canSee) {
                    playerVisible = false;          // memory expires now that we arrived
                    ChangeState(CharacterState::Idle);
                }
            }
        } break;


        case CharacterState::Attack: { //Pirate attack with gun
            stateTimer += deltaTime;
            
            Vector2 myTile = WorldToImageCoords(position);
            Character* occupier = GetTileOccupier(myTile.x, myTile.y, enemyPtrs, this);
            //pirates won't occupy the same tile while shooting. 
            if (occupier && occupier != this) {
                // Only the one with the "greater" pointer backs off
                if (this > occupier) {
                    ChangeState(CharacterState::Reposition);
                    break;
                } else {
                    // Let the other one reposition — wait
                    break;
                }
            }

            // Hysteresis: only leave attack if we're clearly out of range or lost LOS
            if (distance > PIRATE_ATTACK_EXIT || !canSee) {
                ChangeState(CharacterState::Chase);
                break;
            }

            attackCooldown -= deltaTime;
            if (distance < 800 && distance > 350){
                
                if (canSee && attackCooldown <= 0.0f && currentFrame == 1 && !hasFired && type == CharacterType::Pirate) {
                    FireBullet(position, player.position, 1200.0f, 3.0f, true);
                    hasFired = true;
                    attackCooldown = 1.5f;
                    //SoundManager::GetInstance().Play("shotgun");
                    SoundManager::GetInstance().PlaySoundAtPosition("musket", position, player.position, 1.0, 2000);
                }

            }else if (distance < 350){
                ChangeState(CharacterState::MeleeAttack);
            }

            // Wait for next attack opportunity
            if (hasFired && stateTimer > 1.5f) {
                hasFired = false;
                attackCooldown = 3.5f;
                currentFrame = 0;
                stateTimer = 0;

                if (TrySetRandomPatrolPath(start, this, currentWorldPath) && canSee) { //shoot then move to a random tile and shoot again.
                    ChangeState(CharacterState::Patrol);
                }else{
                    ChangeState(CharacterState::Attack);
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
                    Vector2 start = WorldToImageCoords(position);
                    if (TrySetRandomPatrolPath(start, this, currentWorldPath)){
                        ChangeState(CharacterState::Patrol);
                    }else{
                        ChangeState(CharacterState::Idle);

                    }

                } else {
                    ChangeState(CharacterState::MeleeAttack);
                }
                hasFired= false;
                stateTimer = 0.0f;
            }

            break;
        }


        case CharacterState::Reposition: {
            //skeletons and pirates and spiders, when close, surround the player. Instead of all standing on the same tile. 
            stateTimer += deltaTime;

            Vector2 playerTile = WorldToImageCoords(player.position);
            Vector3 target = position; // fallback

            bool foundSpot = FindRepositionTarget(player, position, target);


            if (foundSpot) {
                foundSpot = false;
                Vector3 dir = Vector3Normalize(Vector3Subtract(target, position));
                Vector3 move = Vector3Scale(dir, skeleSpeed * deltaTime);
                position = Vector3Add(position, move);

                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = pirateHeight;

                float dist = Vector3Distance(position, target);

                if (dist < 350.0f && stateTimer > 2.0f) {
                    ChangeState(CharacterState::MeleeAttack);
                } else if (dist > 350.0f && stateTimer > 2.0f) {
                    ChangeState(CharacterState::Chase);
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
                Vector3 move = Vector3Scale(dir, 300 * deltaTime); // slower than chase //maybe make it even slower, pirates are hard to hit. 
                position = Vector3Add(position, move);
                rotationY = RAD2DEG * atan2f(dir.x, dir.z);
                position.y = pirateHeight;

                if (Vector3Distance(position, targetPos) < 100.0f) {
                    currentWorldPath.erase(currentWorldPath.begin());
                }
            }
            else {
                ChangeState(CharacterState::Idle);
            }

            break;
        }

        case CharacterState::Freeze: {
            stateTimer += deltaTime;
            //do nothing

            if (stateTimer > 5.0f){
                ChangeState(CharacterState::Idle);
            }
            break;

        }


        case CharacterState::Stagger: {
            stateTimer += deltaTime;
            //do nothing
           
            if (stateTimer >= 1.0f) {
                canBleed = true;
                ChangeState(CharacterState::Chase);
                
            }
            break;
        }

        case CharacterState::Death:
            if (!isDead) {
                isDead = true;
                deathTimer = 0.0f;         // Start counting
            }

            deathTimer += deltaTime;
            break;
        }
}

bool Character::FindRepositionTarget(const Player& player, const Vector3& selfPos, Vector3& outTarget) {
    //surround the player, don't all stand on the same tile. 
    Vector2 playerTile = WorldToImageCoords(player.position);

    // Get facing direction
    float playerYaw = player.rotation.y;
    Vector3 forward = Vector3Normalize({ sinf(DEG2RAD * playerYaw), 0.0f, cosf(DEG2RAD * playerYaw) });
    Vector3 right = Vector3Normalize(Vector3CrossProduct({ 0, 1, 0 }, forward));

    // Generate offsets
    Vector2 relativeOffsets[3];
    relativeOffsets[0] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(forward, tileSize))) - playerTile;
    relativeOffsets[1] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(right, tileSize))) - playerTile;
    relativeOffsets[2] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(Vector3Negate(right), tileSize))) - playerTile;
    relativeOffsets[3] = WorldToImageCoords(Vector3Add(player.position, Vector3Scale(Vector3Negate(forward), tileSize))) - playerTile;

    for (int i = 0; i < 4; ++i) {
        int tx = (int)playerTile.x + (int)roundf(relativeOffsets[i].x);
        int ty = (int)playerTile.y + (int)roundf(relativeOffsets[i].y);

        if (tx < 0 || ty < 0 || tx >= dungeonWidth || ty >= dungeonHeight) continue;
        if (!IsWalkable(tx, ty, dungeonImg)) continue;
        if (IsTileOccupied(tx, ty, enemyPtrs, nullptr)) continue;

        outTarget = GetDungeonWorldPos(tx, ty, tileSize, dungeonPlayerHeight);
        outTarget.y += 80.0f;
        return true;
    }

    return false;
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

void Character::SetPath(Vector2 start){
    float pirateHeight = 160;
    Vector2 goal = WorldToImageCoords(player.position);
    std::vector<Vector2> tilePath = SmoothPath(FindPath(start, goal), dungeonImg);

    currentWorldPath.clear(); //construct the path the frame before chasing
    for (const Vector2& tile : tilePath) {
        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
        worldPos.y = 180;
        if (type == CharacterType::Pirate) worldPos.y = pirateHeight;
        currentWorldPath.push_back(worldPos);
    }

}

// Optional repulsion: pass a small lateral force in world units/sec (e.g., from ComputeRepulsionForce).
// Leave default {} to behave exactly like before.
bool Character::MoveAlongPath(std::vector<Vector3>& path,
                              Vector3& pos, float& yawDeg,
                              float speed, float dt,
                              float arriveEps,
                              Vector3 repulsion)   // <-- new, optional
{
    if (path.empty()) return false;

    const Vector3 target = path.front();
    Vector3 delta = Vector3Subtract(target, pos);
    const float dist = sqrtf(delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
    if (dist <= arriveEps) {
        path.erase(path.begin());
        return path.empty();
    }

    const float step = fminf(speed * dt, dist);
    Vector3 dir = { delta.x / dist, delta.y / dist, delta.z / dist };

    // Forward move toward waypoint
    Vector3 moveFwd = Vector3Scale(dir, step);

    // Optional repulsion (scaled by dt so it's “units per second”)
    Vector3 moveRep = Vector3Scale(repulsion, dt);

    // Apply both
    Vector3 move = Vector3Add(moveFwd, moveRep);
    pos = Vector3Add(pos, move);

    // Yaw from actual motion if we had any; fallback to forward dir
    const float mxz2 = move.x*move.x + move.z*move.z;
    if (mxz2 > 1e-4f) {
        yawDeg = RAD2DEG * atan2f(move.x, move.z);
    } else {
        yawDeg = RAD2DEG * atan2f(dir.x, dir.z);
    }

    // Snap feet to path height (keep your existing behavior)
    pos.y = target.y;

    return false;
}


// Character.cpp
void Character::SetPathTo(const Vector3& goalWorld) {
    Vector2 start = WorldToImageCoords(position);
    Vector2 goal  = WorldToImageCoords(goalWorld);

    std::vector<Vector2> tilePath = SmoothPath(FindPath(start, goal), dungeonImg);

    currentWorldPath.clear();
    currentWorldPath.reserve(tilePath.size());

    float feetY = (type == CharacterType::Pirate) ? 160.0f : 180.0f; // keep your heights

    for (const Vector2& tile : tilePath) {
        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
        worldPos.y = feetY;
        currentWorldPath.push_back(worldPos);
    }
}


