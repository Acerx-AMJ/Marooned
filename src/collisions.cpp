#include "raylib.h"
#include "bullet.h"
#include "world.h"
#include "dungeonGeneration.h"
#include "sound_manager.h"

#include "resourceManager.h"
#include "raymath.h"
#include "pathfinding.h"




bool CheckCollisionPointBox(Vector3 point, BoundingBox box) {
    return (
        point.x >= box.min.x && point.x <= box.max.x &&
        point.y >= box.min.y && point.y <= box.max.y &&
        point.z >= box.min.z && point.z <= box.max.z
    );
}

void ResolveBoxSphereCollision(const BoundingBox& box, Vector3& position, float radius) {
    // Clamp player position to the inside of the box
    float closestX = Clamp(position.x, box.min.x, box.max.x);
    float closestY = Clamp(position.y, box.min.y, box.max.y);
    float closestZ = Clamp(position.z, box.min.z, box.max.z);

    Vector3 closestPoint = { closestX, closestY, closestZ };
    Vector3 pushDir = Vector3Subtract(position, closestPoint);
    float distance = Vector3Length(pushDir);

    if (distance == 0.0f) {
        // If player is exactly on the box surface, push arbitrarily
        pushDir = {1.0f, 0.0f, 0.0f};
        distance = 0.001f;
    }

    float overlap = radius - distance;
    if (overlap > 0.0f) {
        Vector3 correction = Vector3Scale(Vector3Normalize(pushDir), overlap);
        position = Vector3Add(position, correction);
    }
}

Vector3 ComputeOverlapVector(BoundingBox a, BoundingBox b) {
    float xOverlap = fmin(a.max.x, b.max.x) - fmax(a.min.x, b.min.x);
    float yOverlap = fmin(a.max.y, b.max.y) - fmax(a.min.y, b.min.y);
    float zOverlap = fmin(a.max.z, b.max.z) - fmax(a.min.z, b.min.z);

    if (xOverlap <= 0 || yOverlap <= 0 || zOverlap <= 0) {
        return {0, 0, 0}; // No collision
    }

    float minOverlap = xOverlap;
    Vector3 axis = {1, 0, 0};
    if (yOverlap < minOverlap) {
        minOverlap = yOverlap;
        axis = {0, 1, 0};
    }
    if (zOverlap < minOverlap) {
        minOverlap = zOverlap;
        axis = {0, 0, 1};
    }

    // Get direction from a to b to know which way to push
    Vector3 direction = Vector3Normalize(Vector3Subtract(b.min, a.min));
    float sign = (Vector3DotProduct(axis, direction) >= 0) ? 1.0f : -1.0f;

    return Vector3Scale(axis, minOverlap * sign);
}


void ResolvePlayerEnemyMutualCollision(Character* enemy, Player* player) {
    BoundingBox enemyBox = enemy->GetBoundingBox();
    BoundingBox playerBox = player->GetBoundingBox();

    Vector3 overlap = ComputeOverlapVector(enemyBox, playerBox);
    if (Vector3Length(overlap) > 0) {
        Vector3 correction = Vector3Scale(overlap, 0.5f);
        enemy->position = Vector3Subtract(enemy->position, correction);
        player->position = Vector3Add(player->position, correction);
    }
}

// returns true if a regular (non-AoE) bullet should stop after this pass
bool HandleBarrelHitsForBullet(Bullet& b, Camera& camera) {
    const bool isAOE = (b.type == BulletType::Fireball || b.type == BulletType::Iceball);
    bool hitAnything = false;


    for (BarrelInstance& barrel : barrelInstances) {
        if (barrel.destroyed) continue;

        if (CheckCollisionBoxSphere(barrel.bounds, b.GetPosition(), b.GetRadius())) {
            hitAnything = true;

            // Mark and open the tile
            barrel.destroyed = true;
            int tileX = GetDungeonImageX(barrel.position.x, tileSize, dungeonWidth);
            int tileY = GetDungeonImageY(barrel.position.z, tileSize, dungeonHeight);

            if (tileX >= 0 && tileX < dungeonWidth &&
                tileY >= 0 && tileY < dungeonHeight)
            {
                if (!walkable[tileX][tileY]) {
                    walkable[tileX][tileY] = true;
                }
            }



            // Play SFX
            SoundManager::GetInstance().Play("barrelBreak");


            Vector3 dropPos{ barrel.position.x, barrel.position.y + 100.0f, barrel.position.z };
            if (barrel.containsPotion) {
                collectables.emplace_back(CollectableType::HealthPotion, dropPos, R.GetTexture("healthPotTexture"), 40);
            } else if (barrel.containsMana) {
                collectables.emplace_back(CollectableType::ManaPotion, dropPos, R.GetTexture("manaPotion"), 40);
            } else if (barrel.containsGold) {
                Collectable gold(CollectableType::Gold, dropPos, R.GetTexture("coinTexture"), 40);
                gold.value = GetRandomValue(1, 100);
                collectables.push_back(gold);
            }
            // For non-AoE bullets, stop after the first hit this frame
            if (!isAOE) break;
        }
    }

    // After processing all overlaps, resolve the bullet
    if (hitAnything) {
        if (isAOE) b.Explode(camera);
        else       b.kill(camera);
    }

    // Tell caller whether to stop iterating this bullet
    return hitAnything && !isAOE;
}

void launcherCollision(){
    for (LauncherTrap& launcher : launchers){
        if (CheckCollisionBoxSphere(launcher.bounds, player.position, player.radius)){
            ResolveBoxSphereCollision(launcher.bounds, player.position, player.radius);
        }
    }
}


void SpiderWebCollision(){
    for (SpiderWebInstance& web : spiderWebs){
        if (!web.destroyed && CheckCollisionBoxSphere(web.bounds, player.position, player.radius)){
            ResolveBoxSphereCollision(web.bounds, player.position, player.radius);
        }
    }
}


void DoorCollision(){
    for (Door& door : doors){//player collision
        if (!door.isOpen && CheckCollisionBoxSphere(door.collider, player.position, player.radius)){
           ResolveBoxSphereCollision(door.collider, player.position, player.radius);

        }

        for (Character* enemy : enemyPtrs){ //enemy collilsion 
            if (!door.isOpen && CheckCollisionBoxSphere(door.collider, enemy->position, enemy->radius)){
                ResolveBoxSphereCollision(door.collider, enemy->position, enemy->radius);
            }
        }
        
        //door side colliders
        for (BoundingBox& side : door.sideColliders){
            if (door.isOpen && CheckCollisionBoxSphere(side, player.position, 100)){
                ResolveBoxSphereCollision(side, player.position, 100);
            }

            for (Character* enemy : enemyPtrs){
                if (door.isOpen && CheckCollisionBoxSphere(side, enemy->position, enemy->radius)){
                    ResolveBoxSphereCollision(side, enemy->position, enemy->radius);
                }
            }
        }






    }
}

void WallCollision(){
    for (const WallRun& run : wallRunColliders) { //player wall collision

        for (Character* enemy : enemyPtrs){ //all enemies
            if (CheckCollisionBoxSphere(run.bounds, enemy->position, enemy->radius)){
                ResolveBoxSphereCollision(run.bounds, enemy->position, enemy->radius);
            }
        }

        if (CheckCollisionBoxSphere(run.bounds, player.position, player.radius)) { //player wall collision
            ResolveBoxSphereCollision(run.bounds, player.position, player.radius);
        }


    }
}

void pillarCollision() {
    for (const PillarInstance& pillar : pillars){
        ResolveBoxSphereCollision(pillar.bounds, player.position, player.radius);
        for (Character* enemy : enemyPtrs){
            ResolveBoxSphereCollision(pillar.bounds, enemy->position, enemy->radius);
        }
    }


}

void barrelCollision(){
    
    for (const BarrelInstance& barrel : barrelInstances) {
        if (!barrel.destroyed){ //walk through broke barrels
            ResolveBoxSphereCollision(barrel.bounds, player.position, player.radius);
            for (Character* enemy : enemyPtrs){
                ResolveBoxSphereCollision(barrel.bounds, enemy->position, enemy->radius);
            }
        }
        
    }

}

void ChestCollision(){
    for (const ChestInstance& chest : chestInstances){
        ResolveBoxSphereCollision(chest.bounds, player.position, player.radius);
        for (Character* enemy : enemyPtrs){
            ResolveBoxSphereCollision(chest.bounds, enemy->position, enemy->radius);
        }
    }
}

void HandleEnemyPlayerCollision(Player* player) {
    for (Character* enemy : enemyPtrs) {
        if (enemy->isDead) continue;
        if (CheckCollisionBoxes(enemy->GetBoundingBox(), player->GetBoundingBox())) {
            ResolvePlayerEnemyMutualCollision(enemy, player);
        }
    }
}






void HandleMeleeHitboxCollision(Camera& camera) {

    for (BarrelInstance& barrel : barrelInstances){
        if (barrel.destroyed) continue;
        int tileX = GetDungeonImageX(barrel.position.x, tileSize, dungeonWidth);
        int tileY = GetDungeonImageY(barrel.position.z, tileSize, dungeonHeight);

        if (CheckCollisionBoxes(barrel.bounds, player.meleeHitbox)){
            barrel.destroyed = true;
            walkable[tileX][tileY] = true; //tile is now walkable for enemies
            SoundManager::GetInstance().Play("barrelBreak");
            if (barrel.containsPotion) {
                Vector3 pos = {barrel.position.x, barrel.position.y + 100, barrel.position.z};
                collectables.push_back(Collectable(CollectableType::HealthPotion, pos, R.GetTexture("healthPotTexture"), 40));

            }

            if (barrel.containsGold) {
                Vector3 pos = {barrel.position.x, barrel.position.y + 100, barrel.position.z};
                int gvalue = GetRandomValue(1, 100);
                Collectable gold = Collectable(CollectableType::Gold, pos, R.GetTexture("coinTexture"), 40);
                gold.value = gvalue;
                collectables.push_back(gold);

            }

            if (barrel.containsMana) {
                Vector3 pos = {barrel.position.x, barrel.position.y + 100, barrel.position.z};
                Collectable manaPot = Collectable(CollectableType::ManaPotion, pos, R.GetTexture("manaPotion"), 40);
                collectables.push_back(manaPot);

            }

        }
    }

    for (Character* enemy : enemyPtrs){ //iterate all enemyPtrs
        if (enemy->isDead) continue;
        if (enemy->hitTimer > 0.0f) continue;

        if (CheckCollisionBoxes(enemy->GetBoundingBox(), player.meleeHitbox)){
            enemy->TakeDamage(50);
            if (enemy->type != CharacterType::Skeleton && enemy->type != CharacterType::Ghost){ //skeles and ghosts dont bleed.  
                if (enemy->currentHealth <= 0){
                    meleeWeapon.model.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = R.GetTexture("swordBloody");
                    //spawning decals here doesn't work for whatever reason

                    Vector3 camDir = Vector3Normalize(Vector3Subtract(enemy->position, camera.position));
                    Vector3 offsetPos = Vector3Add(enemy->position, Vector3Scale(camDir, -100.0f));
                    offsetPos.y += 10; 
                    decals.emplace_back(offsetPos, DecalType::Blood, R.GetTexture("bloodSheet"), 7, 0.7f, 0.1f, 60.0f);
                    
                    
                } 
            }
            if (player.activeWeapon == WeaponType::Sword) SoundManager::GetInstance().Play("swordHit");
            if (player.activeWeapon == WeaponType::MagicStaff) SoundManager::GetInstance().Play("staffHit");
        }
    }

    for (SpiderWebInstance& web : spiderWebs){
        if (!web.destroyed && CheckCollisionBoxes(web.bounds, player.meleeHitbox)){
            web.destroyed = true;
            //play a sound
        }
    }


}

void CheckBulletHits(Camera& camera) {
    
    for (Bullet& b : activeBullets) {
        if (!b.IsAlive()) continue;

        Vector3 pos = b.GetPosition();

        // 🔹 1. Hit player
        if (CheckCollisionBoxSphere(player.GetBoundingBox(), b.GetPosition(), b.GetRadius())) { //use CollisionBoxSphere and use bullet radius
            if (b.type == BulletType::Fireball){
                b.Explode(camera);
                //damage delt elseware
                continue;
            }
            if (b.IsEnemy()) {
                b.kill(camera);
                player.TakeDamage(25);
                continue;
            }
        }

        // 🔹 2. Hit enemy
        for (Character* enemy : enemyPtrs) {
            if (enemy->isDead) continue;
            bool isSkeleton = (enemy->type == CharacterType::Skeleton);
            bool isGhost = (enemy->type == CharacterType::Ghost);
            if (CheckCollisionBoxSphere(enemy->GetBoundingBox(), b.GetPosition(), b.GetRadius())) {
                if (!b.IsEnemy() && (b.type == BulletType::Default)) {
                    enemy->TakeDamage(25);

                    if (enemy->isDead) {
                        if (isSkeleton || isGhost){
                            //nothing
                        }else{
                             b.Blood(camera);  // blood decals on death
                        }
                        b.kill(camera);
                    } else {
                        b.kill(camera);
                    }

                    break;
                }
                
                else if (!b.IsEnemy() && (b.type == BulletType::Fireball)){
                    enemy->TakeDamage(25);
                    
                    b.pendingExplosion = true;
                    b.explosionTimer = 0.04f; // short delay
                    // Don't call b.Explode() yet //called in updateFireball

                }else if (!b.IsEnemy() && (b.type == BulletType::Iceball)){
                    //enemy->TakeDamage(25);
                    enemy->state = CharacterState::Freeze;
                    b.pendingExplosion = true;
                    b.explosionTimer = 0.04f;

                    
                } else if (b.IsEnemy() && isSkeleton) { // friendly fire
                    enemy->TakeDamage(25);
                    b.kill(camera);
                    break;
                }

                
            }
        }



        // 🔹 3. Hit walls
        for (WallRun& w : wallRunColliders) {
            if (CheckCollisionPointBox(pos, w.bounds)) {
                if (b.type == BulletType::Fireball || b.type == BulletType::Iceball){
                    b.Explode(camera);
                    break;
                }else{
                    b.kill(camera);
                    break;

                }

            }
        }

        // 🔹 4. Hit doors
        for (Door& d : doors) {
            if (!d.isOpen && CheckCollisionPointBox(pos, d.collider)) {
                if (b.type == BulletType::Fireball || b.type == BulletType::Iceball){
                    b.Explode(camera);
                    break;
                }else{
                    b.kill(camera);
                    break;

                }
            }
            //archway side colliders
            for (BoundingBox& side : d.sideColliders){
                if (d.isOpen && CheckCollisionPointBox(pos, side)){
                    if (b.type == BulletType::Fireball || b.type == BulletType::Iceball){
                        b.Explode(camera);
                        break;
                    }else{
                        b.kill(camera);
                        break;

                    }
                }
            }
        }



        // 🔹 6. Hit pillars
        for (PillarInstance& pillar : pillars) {
            if (CheckCollisionPointBox(pos, pillar.bounds)) {
                if (b.type == BulletType::Fireball || b.type == BulletType::Iceball){
                    b.Explode(camera);
                    break;
                }else{
                    b.kill(camera);
                    break;

                }
            }
        }
        //Hit spiderweb
        for (SpiderWebInstance& web : spiderWebs){
            if (!web.destroyed && CheckCollisionBoxSphere(web.bounds, b.GetPosition(), b.GetRadius())){
                web.destroyed = true;
                if (b.type == BulletType::Fireball || b.type == BulletType::Iceball){
                    b.Explode(camera);
                    break;
                }else{
                    b.kill(camera);
                    break;

                }
            }
        }

                            // bullet hits barrel
        if (HandleBarrelHitsForBullet(b, camera)){
            break; //check bullets last
        }


    }
}

bool CheckBulletHitsTree(const TreeInstance& tree, const Vector3& bulletPos) {
    Vector3 treeBase = {
        tree.position.x + tree.xOffset,
        tree.position.y + tree.yOffset,
        tree.position.z + tree.zOffset
    };

    // Check vertical overlap
    if (bulletPos.y < treeBase.y || bulletPos.y > treeBase.y + tree.colliderHeight) {
        return false;
    }

    // Check horizontal distance from tree trunk center
    float dx = bulletPos.x - treeBase.x;
    float dz = bulletPos.z - treeBase.z;
    float horizontalDistSq = dx * dx + dz * dz;

    return horizontalDistSq <= tree.colliderRadius * tree.colliderRadius;
}

bool CheckTreeCollision(const TreeInstance& tree, const Vector3& playerPos) {
    Vector3 treeBase = {
        tree.position.x + tree.xOffset,
        tree.position.y + tree.yOffset,
        tree.position.z + tree.zOffset
    };

    float dx = playerPos.x - treeBase.x;
    float dz = playerPos.z - treeBase.z;
    float horizontalDistSq = dx * dx + dz * dz;

    if (horizontalDistSq < tree.colliderRadius * tree.colliderRadius &&
        playerPos.y >= treeBase.y &&
        playerPos.y <= treeBase.y + tree.colliderHeight) {
        return true;
    }

    return false;
}

void ResolveTreeCollision(const TreeInstance& tree, Vector3& playerPos) {
    Vector3 treeBase = {
        tree.position.x + tree.xOffset,
        tree.position.y + tree.yOffset,
        tree.position.z + tree.zOffset
    };

    float dx = playerPos.x - treeBase.x;
    float dz = playerPos.z - treeBase.z;
    float distSq = dx * dx + dz * dz;

    float radius = tree.colliderRadius;
    if (distSq < radius * radius) {
        float dist = sqrtf(distSq);
        float overlap = radius - dist;

        if (dist > 0.01f) {
            dx /= dist;
            dz /= dist;
            playerPos.x += dx * overlap;
            playerPos.z += dz * overlap;
        }
    }
}



void CheckBulletsAgainstTrees(std::vector<TreeInstance>& trees,
                              Camera& camera)
{
    constexpr float CULL_RADIUS = 500.0f;
    const float cullDistSq = CULL_RADIUS * CULL_RADIUS;

    for (TreeInstance& tree : trees) {
        // use the same center the collision routine uses
        Vector3 treeBase = {
            tree.position.x + tree.xOffset,
            tree.position.y + tree.yOffset,
            tree.position.z + tree.zOffset
        };

        

        for (Bullet& bullet : activeBullets) {
            if (!bullet.IsAlive()) continue;

            const Vector3 bp = bullet.GetPosition();

            // cheap early-out around the correct center
            if (Vector3DistanceSqr(treeBase, bp) < cullDistSq) {
                if (CheckBulletHitsTree(tree, bp)) {
                    if (bullet.type == BulletType::Fireball){
                        bullet.Explode(camera);

                    }else{
                        bullet.kill(camera);
                    } 

                    break; // stop checking this tree for this frame
                }
            }
        }
    }
}



void TreeCollision(Camera& camera){

    for (TreeInstance& tree : trees) {
        if (Vector3DistanceSqr(tree.position, player.position) < 500 * 500) { //check a smaller area not the whole map. 
            if (CheckTreeCollision(tree, player.position)) {
                ResolveTreeCollision(tree, player.position);
            }
        }
    }

    for (Character* enemy : enemyPtrs){
        for (TreeInstance& tree : trees) {
            if (Vector3DistanceSqr(tree.position, enemy->position) < 500*500) {
                if (CheckTreeCollision(tree, enemy->position)) {
                    ResolveTreeCollision(tree, enemy->position);
                    
                }
            }
        }

    }


    CheckBulletsAgainstTrees(trees,  camera);


}



void HandleDoorInteraction(Camera& camera) {
    static bool isWaiting = false;
    static float openTimer = 0.0f;
    static int pendingDoorIndex = -1;

    float deltaTime = GetFrameTime();

    if (!isWaiting && IsKeyPressed(KEY_E)) {
        for (size_t i = 0; i < doors.size(); ++i) {
            float distanceTo = Vector3Distance(doors[i].position, player.position);
            if (distanceTo < 300) {

                // If locked and no key, deny access
                if (doors[i].isLocked) {
                    if (player.inventory.HasItem("GoldKey")) {
                        player.inventory.UseItem("GoldKey");
                        doors[i].isLocked = false;
                        SoundManager::GetInstance().Play("unlock");
                    } else {
                        SoundManager::GetInstance().Play("lockedDoor");
                        return; // skip the rest of this function
                    }
                }

                // Start door interaction (fade, open, etc)
                isWaiting = true;
                openTimer = 0.0f;
                pendingDoorIndex = i;

                std::string s = doors[i].isOpen ? "doorClose" : "doorOpen";
                SoundManager::GetInstance().Play(s);

                DoorType type = doors[i].doorType;
                if (type == DoorType::GoToNext || type == DoorType::ExitToPrevious) {
                    previousLevelIndex = levelIndex;
                    isWaiting = false;
                    openTimer = 0.0f;
                    //fadeIn = true;
                    //isFading = true;
                    pendingLevelIndex = doors[i].linkedLevelIndex;
                    StartFadeOutToLevel(pendingLevelIndex);
                    isLoadingLevel = true; //stops updating characters, prevents crash on level switch.
                    currentGameState = GameState::LoadingLevel; 
                    if (levelIndex == 4) unlockEntrances = true;

                }

                break; // done with this door
            }
        }
    }


    if (isWaiting) {
        openTimer += deltaTime;

        if (openTimer >= 0.5f && pendingDoorIndex != -1) {
            doors[pendingDoorIndex].isOpen = !doors[pendingDoorIndex].isOpen;
            doorways[pendingDoorIndex].isOpen = doors[pendingDoorIndex].isOpen;

            // Update walkable grid, open doors are walkable. 
            int tileX = GetDungeonImageX(doors[pendingDoorIndex].position.x, tileSize, dungeonWidth);
            int tileY = GetDungeonImageY(doors[pendingDoorIndex].position.z, tileSize, dungeonHeight);
            if (tileX >= 0 && tileY >= 0 && tileX < walkable.size() && tileY < walkable[0].size()) {
                walkable[tileX][tileY] = doors[pendingDoorIndex].isOpen;
            }

            // Reset
            isWaiting = false;
            openTimer = 0.0f;
            pendingDoorIndex = -1;
        }
    }
}

void UpdateCollisions(Camera& camera){
    CheckBulletHits(camera); //bullet collision
    TreeCollision(camera); //player and raptor vs tree
    WallCollision();
    DoorCollision();
    SpiderWebCollision();
    barrelCollision();
    ChestCollision();
    HandleEnemyPlayerCollision(&player);
    pillarCollision();
    launcherCollision();
    HandleMeleeHitboxCollision(camera);
}


