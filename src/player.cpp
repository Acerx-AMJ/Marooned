#include "player.h"
#include "raymath.h"
#include "world.h"
#include <iostream>
#include "resourceManager.h"
#include "input.h"
#include "boat.h"
#include "rlgl.h"
#include "sound_manager.h"
#include "inventory.h"
#include "input.h"
#include "weapon.h"
#include "camera_system.h"
#include "array"

Weapon weapon;
MeleeWeapon meleeWeapon;
MagicStaff magicStaff;


//
void InitPlayer(Player& player, Vector3 startPosition) {

    player.position = startPosition;
    player.startPosition = startPosition;
    
    player.rotation.y = levels[levelIndex].startingRotationY;
    player.velocity = {0, 0, 0};
    player.grounded = false;
    player.groundY = 0.0;

    InitBlunderbuss(weapon);
    InitSword(meleeWeapon);
    InitMagicStaff(magicStaff);

    player.inventory.SetupItemTextures();
    playerInit = true;

    if (first){
        first = false; // player first starting position uses first as well, it's set to false here
        player.inventory.AddItem("HealthPotion");
    
    }
    
}


void Player::EquipNextWeapon() {
    meleeWeapon.model.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = R.GetTexture("swordClean"); //wipe the blood off. 
    if (collectedWeapons.empty()) {
       
        activeWeapon = WeaponType::None;
        currentWeaponIndex = -1;
        return;
    }
    
    currentWeaponIndex = (currentWeaponIndex + 1) % collectedWeapons.size();
    activeWeapon = collectedWeapons[currentWeaponIndex];
}



void HandleKeyboardInput(float deltaTime) {
    if (!player.canMove) return;
    player.isMoving = false;
    Vector3 input = {0};
    if (IsKeyDown(KEY_W)) input.z += 1;
    if (IsKeyDown(KEY_S)) input.z -= 1;
    if (IsKeyDown(KEY_A)) input.x += 1;
    if (IsKeyDown(KEY_D)) input.x -= 1;

    player.running = IsKeyDown(KEY_LEFT_SHIFT) && player.canRun;
    float speed = player.running ? player.runSpeed : player.walkSpeed;

    if (input.x != 0 || input.z != 0) {
        input = Vector3Normalize(input);
        float yawRad = DEG2RAD * player.rotation.y;
        player.isMoving = true;
        Vector3 forward = { sinf(yawRad), 0, cosf(yawRad) };
        Vector3 right = { forward.z, 0, -forward.x };

        Vector3 moveDir = {
            forward.x * input.z + right.x * input.x,
            0,
            forward.z * input.z + right.z * input.x
        };

        moveDir = Vector3Scale(Vector3Normalize(moveDir), speed * deltaTime);
        player.position = Vector3Add(player.position, moveDir);
        player.forward = forward;
    }

    if (player.grounded && IsKeyPressed(KEY_SPACE)) {
        player.velocity.y = player.jumpStrength;
        player.grounded = false;
    }

    if (IsKeyPressed(KEY_Q)) {
 
        player.EquipNextWeapon();
    }


    if (IsKeyPressed(KEY_ONE)){
        //use health potion
        if (player.inventory.HasItem("HealthPotion")){
            
            if (player.currentHealth < player.maxHealth){
                player.currentHealth = player.maxHealth;
                player.inventory.UseItem("HealthPotion");
                SoundManager::GetInstance().Play("gulp");
            }
        }
    }

    if (IsKeyPressed(KEY_TWO)){
        if (player.inventory.HasItem("ManaPotion")){
            if (player.currentMana < player.maxMana){
                player.currentMana = player.maxMana;
                player.inventory.UseItem("ManaPotion");
                SoundManager::GetInstance().Play("gulp");
            }
        }
    }

    if (IsKeyPressed(KEY_T)){
       if (magicStaff.magicType == MagicType::Fireball){
            magicStaff.magicType = MagicType::Iceball;
       }else{
            magicStaff.magicType = MagicType::Fireball;
       }
    }

}


void UpdateBlockHitbox(Player& player, float blockDistance = 500.0f, float width = 300.0f, float height = 64.0f) {
    //BlockHitbox is a large rectange that covers an area in front of the player. It is active when blocking. If enemies are inside the rectangle
    //there attacks will be blocked. 
    if (!player.blocking) return;

    Vector3 forward = {
        sinf(DEG2RAD * player.rotation.y),
        0,
        cosf(DEG2RAD * player.rotation.y)
    };

    Vector3 center = Vector3Add(player.position, Vector3Scale(forward, blockDistance));
    center.y = player.position.y;

    player.blockHitbox.min = {
        center.x - width / 2.0f,
        center.y - height / 2.0f,
        center.z - width / 2.0f
    };
    player.blockHitbox.max = {
        center.x + width / 2.0f,
        center.y + height / 2.0f,
        center.z + width / 2.0f
    };
}

BoundingBox Player::GetBoundingBox() const {
    float halfWidth = (200* 0.5f * 0.4f) / 2.0f; 
    float halfHeight = (200 * 0.5f) / 2.0f;

    return {
        { player.position.x - halfWidth, player.position.y - halfHeight, player.position.z - halfWidth },
        { player.position.x + halfWidth, player.position.y + halfHeight, player.position.z + halfWidth }
    };
}

// void PlaySwimSound() {
//     if (!player.isSwimming){

//         return;

//     } 
//     static std::vector<std::string> swimKeys = { "swim1", "swim2", "swim3", "swim4" };
//     static int lastIndex = -1;

//     int index;
//     do {
//         index = GetRandomValue(0, swimKeys.size() - 1);
//     } while (index == lastIndex && swimKeys.size() > 1);  // avoid repeat if more than 1

//     lastIndex = index;
//     std::string swimKey = swimKeys[index];

//     SoundManager::GetInstance().Play(swimKey);
    
// }

void PlaySwimOnce()
{
    static const std::array<const char*,4> KEYS = { "swim1","swim2","swim3","swim4" };
    static int lastIndex = -1;
    static Sound current = {0};  // raylib Sound handle of the *last* played clip

    // If a previous swim sound is still playing, do nothing
    if (current.frameCount > 0 && IsSoundPlaying(current)){
         return;

    }

    int idx;
    do { idx = GetRandomValue(0, (int)KEYS.size() - 1); }
    while (idx == lastIndex && KEYS.size() > 1);
    lastIndex = idx;

    current = SoundManager::GetInstance().GetSound(KEYS[idx]);
    PlaySound(current);
}




void PlayFootstepSound() {
    if (player.isSwimming) return;
    static std::vector<std::string> footstepKeys = { "step1", "step2", "step3", "step4" };
    static int lastIndex = -1;

    int index;
    do {
        index = GetRandomValue(0, footstepKeys.size() - 1);
    } while (index == lastIndex && footstepKeys.size() > 1);  // avoid repeat if more than 1

    lastIndex = index;
    std::string stepKey = footstepKeys[index];

    SoundManager::GetInstance().Play(stepKey);
}

void UpdateFootsteps(float deltaTime){

    if (player.isMoving && player.grounded && !player.onBoard) {
        player.footstepTimer += deltaTime;

        float interval = player.running ? 0.4f : 0.6f;

        if (player.footstepTimer >= interval) {
            PlayFootstepSound();
            player.footstepTimer = 0.0f;
        }
    } else {
        player.footstepTimer = 0.0f;
    }

}

void UpdateSwimSounds(float deltaTime){
    if (!player.isSwimming) return;
    if (player.isMoving && player.isSwimming && !player.onBoard) {
        player.swimTimer += deltaTime;
        float interval = 0.9f;

        if (player.swimTimer >= interval) {
            PlaySwimOnce();
            player.swimTimer = 0.0f;
        }
    } else {
        player.swimTimer = 0.0f;
    }

}

void UpdateMeleeHitbox(Camera& camera){
    if (meleeWeapon.hitboxActive || magicStaff.hitboxActive) {
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, { 0, 1, 0 }));

        Vector3 hitboxCenter = Vector3Add(player.position, Vector3Scale(forward, 200.0f));
        hitboxCenter.y += 0.0f; 

        Vector3 boxSize = {100.0f, 100.0f, 100.0f};

        Vector3 min = {
            hitboxCenter.x - boxSize.x * 0.5f,
            hitboxCenter.y - boxSize.y * 0.5f,
            hitboxCenter.z - boxSize.z * 0.5f
        };
        Vector3 max = {
            hitboxCenter.x + boxSize.x * 0.5f,
            hitboxCenter.y + boxSize.y * 0.5f,
            hitboxCenter.z + boxSize.z * 0.5f
        };

        player.meleeHitbox = { min, max };
    } else {
        // Collapse the hitbox to prevent accidental damage
        player.meleeHitbox = { player.position, player.position };
    }
}

void InitSword(MeleeWeapon& meleeWeapon){
    //init sword
    meleeWeapon.model = R.GetModel("swordModel");
    meleeWeapon.scale = {2, 2, 2};
    meleeWeapon.model.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = R.GetTexture("swordClean");
    
}

void InitBlunderbuss(Weapon& weapon){
    //init blunderbuss
    weapon.model = R.GetModel("blunderbuss");
    weapon.scale = { 2.0f, 2.0f, 2.0f };
    weapon.muzzleFlashTexture = R.GetTexture("muzzleFlash");
    weapon.fireCooldown = 2.0f;
    weapon.flashDuration = 1.0;

}

void InitMagicStaff(MagicStaff& magicStaff) {
    magicStaff.model = R.GetModel("staffModel");
    magicStaff.scale = {1.0f, 1.0f, 1.0f};
    magicStaff.muzzleFlashTexture = R.GetTexture("muzzleFlash");
    magicStaff.fireCooldown = 1.0f;

    //set starting offsets here
    magicStaff.forwardOffset = 75.0f;
    magicStaff.sideOffset = 28.0f;
    magicStaff.verticalOffset = -25.0f;

   
}



void UpdatePlayer(Player& player, float deltaTime, Camera& camera) {

    weapon.Update(deltaTime);
    weapon.isMoving = player.isMoving;
    meleeWeapon.isMoving = player.isMoving;
    magicStaff.isMoving = player.isMoving;
    meleeWeapon.Update(deltaTime);
    magicStaff.Update(deltaTime);
    UpdateMeleeHitbox(camera);
    UpdateFootsteps(deltaTime);

    UpdateBlockHitbox(player, 250, 300, 100);
    vignetteFade += deltaTime * 2.0f; 
    vignetteIntensity = Clamp(1.0f - vignetteFade, 0.0f, 1.0f);

    if (player.hitTimer > 0.0f){
        player.hitTimer -= deltaTime;
        camera.fovy = Lerp(camera.fovy, player.targetFOV, deltaTime * 12.0f); // fast zoom
    }else{
        player.hitTimer = 0;
        camera.fovy = Lerp(camera.fovy, 45, deltaTime * 4.0f); // smooth return
    }

    float goldLerpSpeed = 5.0f;
    player.displayedGold += (player.gold - player.displayedGold) * goldLerpSpeed * deltaTime;

    if (player.running && player.isMoving && player.grounded && player.stamina > 0.0f) {
        player.stamina -= deltaTime * 30.0f; // drain rate
        if (player.stamina <= 0.0f) {
            player.stamina = 0.0f;
            player.canRun = false; // forced stop
        }
    }
    else {
        // Recover stamina
        player.stamina += deltaTime * 20.0f; // regen rate
        if (player.stamina > 0.0f) {
            player.canRun = true; // can run as soon as some stamina is back
        }
        if (player.stamina >= player.maxStamina) {
            player.stamina = player.maxStamina;
        }
    }

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {

        if (player.activeWeapon == WeaponType::Sword){
            player.blocking = true; //only block with the sword
            meleeWeapon.StartBlock();
        }else if (player.activeWeapon == WeaponType::MagicStaff){
            magicStaff.Fire(camera);
        }
    } else {
        meleeWeapon.EndBlock();
        player.blocking = false;
    }



    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!player.isSwimming){
           if (player.activeWeapon == WeaponType::Blunderbuss){
                weapon.Fire(camera); 
           } else if (player.activeWeapon == WeaponType::Sword){
                meleeWeapon.StartSwing();
           } else if (player.activeWeapon == WeaponType::MagicStaff){
                magicStaff.StartSwing();
           }
           
           
        }else{
            if (player.activeWeapon == WeaponType::Blunderbuss) SoundManager::GetInstance().Play("reload"); //play "click" if in water with gun
        }
        
    }





    // --- Boarding Check ---
    if (!player.onBoard) { //board the boat, lock player position to boat position, keep free look
        float distanceToBoat = Vector3Distance(player.position, player_boat.position);
        if (distanceToBoat < 300.0f && IsKeyPressed(KEY_SPACE)) {
            player.onBoard = true;
            player_boat.playerOnBoard = true;
            player.position = Vector3Add(player_boat.position, {0, 200.0f, 0}); // sit up a bit
            return; // skip rest of update this frame
        }
    }

    // --- Exit Boat ---
    if (player.onBoard && IsKeyPressed(KEY_SPACE)) {
        player.onBoard = false;
        player_boat.playerOnBoard = false;
        player.position = Vector3Add(player_boat.position, {2.0f, 0.0f, 0.0f}); // step off
    }

    // --- Sync Player to Boat ---
    if (player.onBoard) {
        player.position = Vector3Add(player_boat.position, {0, 200.0f, 0});
    }

    // === Swimming Check ===
    if (player.position.y <= waterHeightY + player.height / 2.0f) {
        player.isSwimming = true;
        UpdateSwimSounds(deltaTime);
    } else {
        player.isSwimming = false;
    }


    HandleMouseLook(deltaTime);
    

    // === Skip Movement if On Boat ===
    if (player.onBoard) {
        return;
    }

    // === Gravity ===
    if (!player.grounded) {
        player.velocity.y -= player.gravity * deltaTime;
        player.position.y += player.velocity.y * deltaTime;
    }

    // === Ground Check ===
    
    player.groundY = GetHeightAtWorldPosition(player.position, heightmap, terrainScale);
    
    if (isDungeon) {
        player.groundY = dungeonPlayerHeight;
        if (player.overLava) {
            player.groundY -= 150.0f; // or LAVA_DROP
        }
    }
    float feetY = player.position.y - player.height / 2.0f;

    if (feetY <= player.groundY + 5.0f) { //+5 buffer for uneven terrain. 
        player.grounded = true;
        player.velocity.y = 0;
        player.position.y = player.groundY + player.height / 2.0f;
    } else {
        player.grounded = false;
    }

    // === Jump Input ===
    if (player.grounded) {
        if (IsGamepadAvailable(0)) {
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                player.velocity.y = player.jumpStrength;
                player.grounded = false;
            }
        } else {
            if (IsKeyPressed(KEY_SPACE)) {
                player.velocity.y = player.jumpStrength;
                player.grounded = false;
            }
        }
    }

    //start the dying process. 
    if (player.dying) {
        player.deathTimer += deltaTime;
        player.velocity = {0.0f, 0.0f, 0.0f}; //stop moving when dying. should hide the gun as well. 
        player.canMove = false;
        vignetteIntensity = 1.0f; //should stay red becuase its set to 1 everyframe. 
        vignetteFade = 0.0f;
        isFading = true;
        fadeIn = true;      // fade to black
        fadeSpeed = 1.5f;   

        if (player.deathTimer > 1.5f) { 
            player.dying = false;
            player.dead = true;
        }
    }

    if (player.dead) {
        // Reset position and state
        player.position = player.startPosition;
        player.velocity = {0}; 
        player.currentHealth = player.maxHealth;
        player.dead = false;
        player.canMove = true;

        isFading = true;
        fadeIn = false;     // fade out of black
        fadeSpeed = 1.5f;

    }

    //PLAYER MOVEMENT KEYBOARD INPUT
    if (controlPlayer) HandleKeyboardInput(deltaTime);
   
}

void Player::TakeDamage(int amount){

    if (!player.dying && !player.dead) {
        player.currentHealth -= amount;

        if (player.currentHealth <= 0) {
            player.dying = true;
            player.deathTimer = 0.0f;
           
        }
    }
    hitTimer = 0.15;
    vignetteIntensity = 1.0f;
    vignetteFade = 0.0f;

    if (rand() % 2 == 0){
        SoundManager::GetInstance().Play("phit1");
    }else{
        SoundManager::GetInstance().Play("phit2");
    }

}

void DrawPlayer(const Player& player, Camera& camera) {
    DrawCapsule(player.position, Vector3 {player.position.x, player.height/2, player.position.z}, 5, 4, 4, RED);
    //DrawBoundingBox(player.GetBoundingBox(), RED);
    //DrawBoundingBox(player.meleeHitbox, WHITE);

    if (controlPlayer) {
        switch (player.activeWeapon) {
            case WeaponType::Blunderbuss:
                weapon.Draw(camera);
                break;
            case WeaponType::Sword:
                meleeWeapon.Draw(camera);
                if (meleeWeapon.hitboxActive) {
                    // DrawBoundingBox(player.meleeHitbox, RED);
                }
                if (player.blocking) {
                    // DrawBoundingBox(player.blockHitbox, RED);
                }
                break;
            case WeaponType::MagicStaff:
                magicStaff.Draw(camera);
                break;
            case WeaponType::None:
                // draw nothing
                break;
        }
    }
}


