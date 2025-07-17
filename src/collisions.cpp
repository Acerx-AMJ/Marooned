#include "raylib.h"
#include "bullet.h"
#include "world.h"
#include "dungeonGeneration.h"
#include "sound_manager.h"
#include "resources.h"
#include "raymath.h"


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
        if (CheckCollisionBoxes(barrel.bounds, player.meleeHitbox)){
            barrel.destroyed = true;
            SoundManager::GetInstance().Play("barrelBreak");
            if (barrel.containsPotion) {
                Vector3 pos = {barrel.position.x, barrel.position.y + 100, barrel.position.z};
                collectables.push_back(Collectable(CollectableType::HealthPotion, pos, &healthPotTexture, 40));

            }

            if (barrel.containsGold) {
                Vector3 pos = {barrel.position.x, barrel.position.y + 100, barrel.position.z};
                int gvalue = GetRandomValue(1, 100);
                Collectable gold = Collectable(CollectableType::Gold, pos, &coinTexture, 40);
                gold.value = gvalue;
                collectables.push_back(gold);

            }

        }
    }

    for (Character* enemy : enemyPtrs){ //iterate all enemyPtrs
        if (enemy->isDead) continue;
        if (enemy->hitTimer > 0.0f) continue;

        if (CheckCollisionBoxes(enemy->GetBoundingBox(), player.meleeHitbox) && enemy->hitTimer <= 0){
            enemy->TakeDamage(50);
            if (enemy->type != CharacterType::Skeleton){ //if raptor or pirate, bloody sword on death. 
                if (enemy->currentHealth <= 0){
                    swordModel.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = swordBloody;
                    //spawning decals here doesn't work for whatever reason

                    Vector3 camDir = Vector3Normalize(Vector3Subtract(enemy->position, camera.position));
                    Vector3 offsetPos = Vector3Add(enemy->position, Vector3Scale(camDir, -100.0f));
                    offsetPos.y += 10; 
                    decals.emplace_back(offsetPos, DecalType::Blood, &bloodSheet, 7, 0.7f, 0.1f, 60.0f);
                    
                    
                } 
            }
            SoundManager::GetInstance().Play("swordHit");
        }
    }

    for (SpiderWebInstance& web : spiderWebs){
        if (!web.destroyed && CheckCollisionBoxes(web.bounds, player.meleeHitbox)){
            web.destroyed = true;
        }
    }


}

void CheckBulletHits(Camera& camera) {
    for (Bullet& b : activeBullets) {
        if (!b.IsAlive()) continue;

        Vector3 pos = b.GetPosition();

        // 🔹 1. Hit player
        if (CheckCollisionPointBox(pos, player.GetBoundingBox())) {
            if (b.IsEnemy()) {
                b.kill(camera);
                player.TakeDamage(25);
                continue;
            }
        }

        // 🔹 2. Hit enemy
        for (Character* enemy : enemyPtrs) {
            if (enemy->isDead) continue;

            if (CheckCollisionPointBox(pos, enemy->GetBoundingBox())) {
                if (!b.IsEnemy()) {
                    enemy->TakeDamage(25);
                    
                    if (enemy->type != CharacterType::Skeleton && enemy->currentHealth <= 0){
                        b.Blood(camera); //spawn blood decals on non skeleton death. 
                    }else{
                        b.kill(camera);
                    }

                    break;
                } else if (enemy->type == CharacterType::Skeleton) { // friendly fire
                    enemy->TakeDamage(25);
                    b.kill(camera);
                    break;
                }

                
            }
        }

        // 🔹 3. Hit walls
        for (WallRun& w : wallRunColliders) {
            if (CheckCollisionPointBox(pos, w.bounds)) {
                b.kill(camera);
                break;
            }
        }

        // 🔹 4. Hit doors
        for (Door& d : doors) {
            if (!d.isOpen && CheckCollisionPointBox(pos, d.collider)) {
                b.kill(camera);
                break;
            }
            //archway side colliders
            for (BoundingBox& side : d.sideColliders){
                if (d.isOpen && CheckCollisionPointBox(pos, side)){
                    b.kill(camera);
                    break;
                }
            }
        }

        // 🔹 5. Hit barrels
        for (BarrelInstance& barrel : barrelInstances) {
            if (!barrel.destroyed && CheckCollisionPointBox(pos, barrel.bounds)) {
                barrel.destroyed = true;
                b.kill(camera);
                SoundManager::GetInstance().Play("barrelBreak");

                if (barrel.containsPotion) {
                    Vector3 dropPos = { barrel.position.x, barrel.position.y + 100, barrel.position.z };
                    collectables.push_back(Collectable(CollectableType::HealthPotion, dropPos, &healthPotTexture, 40));
                }

                if (barrel.containsGold) {
                    Vector3 pos = {barrel.position.x, barrel.position.y + 100, barrel.position.z};
                    int gvalue = GetRandomValue(1, 100);
                    Collectable gold(CollectableType::Gold, pos, &coinTexture, 40);
                    gold.value = gvalue;
                    collectables.push_back(gold);

                }
                break;
            }
        }

        // 🔹 6. Hit pillars
        for (PillarInstance& pillar : pillars) {
            if (CheckCollisionPointBox(pos, pillar.bounds)) {
                b.kill(camera);
                break;
            }
        }

        for (SpiderWebInstance& web : spiderWebs){
            if (!web.destroyed && CheckCollisionBoxSphere(web.bounds, b.GetPosition(), 2)){
                web.destroyed = true;
                b.kill(camera);
                break;
            }
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



    for (TreeInstance& tree : trees) {
        for (Bullet& bullet : activeBullets){
            if (!bullet.IsAlive()) continue; // <-- early out for dead bullets
            if (Vector3DistanceSqr(tree.position, bullet.GetPosition()) < 500 * 500) { 
                if (CheckBulletHitsTree(tree, bullet.GetPosition())) {
                   
                   
                    //Tree hit by bullet. Play a sound. 
                    bullet.kill(camera);
                    break;
                }

            }
 
        }
    }

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
                    fadeIn = true;
                    isFading = true;
                    pendingLevelIndex = doors[i].linkedLevelIndex;
                    isLoadingLevel = true; //stops updating characters, prevents crash on level switch. 
                }

                break; // done with this door
            }
        }
    }


    if (isWaiting) {
        openTimer += deltaTime;

        if (openTimer >= 0.5f && pendingDoorIndex != -1) { 
            doors[pendingDoorIndex].isOpen = !doors[pendingDoorIndex].isOpen;//open if closed, close if open. both the archway and the door. 
            doorways[pendingDoorIndex].isOpen = doors[pendingDoorIndex].isOpen;

            // Reset
            isWaiting = false;
            openTimer = 0.0f;
            pendingDoorIndex = -1;
        }
    }
}


