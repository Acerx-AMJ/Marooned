#include "player.h"
#include "raymath.h"
#include "world.h"
#include <iostream>
#include "resources.h"
#include "input.h"
#include "boat.h"
#include "rlgl.h"
#include "sound_manager.h"

Weapon weapon;

void InitPlayer(Player& player, Vector3 startPosition) {
    player.position = startPosition;
    player.startPosition = startPosition;
    player.velocity = {0, 0, 0};
    player.grounded = false;

    weapon.model = blunderbuss;//LoadModel("assets/models/blunderbus.glb"); //shouldn't this be in resources. 
    weapon.scale = { 2.0f, 2.0f, 2.0f };

    weapon.muzzleFlashTexture = muzzleFlash;
    weapon.forwardOffset = 80.0f;
    weapon.sideOffset = 20.0f;
    weapon.verticalOffset = -30.0f;

    weapon.fireCooldown = 2.0f;
    
}

void HandleMouseLook(float deltaTime){
    Vector2 mouseDelta = GetMouseDelta();
    float mouseSensitivity = 0.03f;
    player.rotation.y -= mouseDelta.x * mouseSensitivity;
    player.rotation.x -= mouseDelta.y * mouseSensitivity;
    player.rotation.x = Clamp(player.rotation.x, -89.0f, 89.0f);

}

void HandleKeyboardInput(float deltaTime) {
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
}

void HandleStickLook(float deltaTime){

    float lookSensitivity = 2.0f;
    //float moveSensitivity = 1.0f;

    float yaw = -GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) * lookSensitivity;
    float pitch = -GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * lookSensitivity;

    player.rotation.y += yaw;
    player.rotation.x += pitch;
    player.rotation.x = Clamp(player.rotation.x, -89.0f, 89.0f);

}

void HandleGamepadInput(float deltaTime) {


    Vector2 moveStick = {
        -GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)
    };

    float speed = player.running ? player.runSpeed : player.walkSpeed;

    float yawRad = DEG2RAD * player.rotation.y;
    Vector3 forward = { sinf(yawRad), 0, cosf(yawRad) };
    Vector3 right = { forward.z, 0, -forward.x };

    Vector3 moveDir = {
        forward.x * -moveStick.y + right.x * moveStick.x,
        0,
        forward.z * -moveStick.y + right.z * moveStick.x
    };

    moveDir = Vector3Scale(Vector3Normalize(moveDir), speed * deltaTime);
    player.position = Vector3Add(player.position, moveDir);
    player.forward = forward;

    if (player.grounded && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        player.velocity.y = player.jumpStrength;
        player.grounded = false;
        //std::cout << "jumping\n";
    }
}

void Player::TakeDamage(int amount){
    if (!player.dying && !player.dead) {
        player.currentHealth -= amount;

        if (player.currentHealth <= 0) {
            player.dying = true;
            player.deathTimer = 0.0f;

            //PlaySound(screamSound); // cue the Wilhelm
        }
    }

    vignetteIntensity = 1.0f;
    vignetteFade = 0.0f;

    if (rand() % 2 == 0){
        SoundManager::GetInstance().Play("phit1");
    }else{
        SoundManager::GetInstance().Play("phit2");
    }



}



void PlayFootstepSound() {
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
    if (player.isMoving && player.grounded &&!player.onBoard) {
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




void UpdatePlayer(Player& player, float deltaTime, Mesh terrainMesh, Camera& camera) {
    weapon.Update(deltaTime);
    UpdateFootsteps(deltaTime);
    vignetteFade += deltaTime * 2.0f; // tweak fade speed
    vignetteIntensity = Clamp(1.0f - vignetteFade, 0.0f, 1.0f);



    if (player.running && player.isMoving && player.grounded && player.stamina > 0.0f) {
        player.stamina -= deltaTime * 30.0f; // adjust drain rate
        if (player.stamina <= 0.0f) {
            player.stamina = 0.0f;
            player.canRun = false;
        }
    }
    else {
        // Recover stamina
        player.stamina += deltaTime * 20.0f; // adjust regen rate
        if (player.stamina >= player.maxStamina) {
            player.stamina = player.maxStamina;
            player.canRun = true;
        }
    }




    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!player.isSwimming){
            weapon.Fire(camera);        
        }else{
            SoundManager::GetInstance().Play("reload");
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
    } else {
        player.isSwimming = false;
    }

    // === Camera Look ===
    if (currentInputMode == InputMode::Gamepad) {
        HandleStickLook(deltaTime);
    } else {
        HandleMouseLook(deltaTime);
    }

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
    float groundY = GetHeightAtWorldPosition(player.position, heightmap, terrainScale);
    float feetY = player.position.y - player.height / 2.0f;

    if (feetY <= groundY + 5.0f) { //+5 buffer for uneven terrain. 
        player.grounded = true;
        player.velocity.y = 0;
        player.position.y = groundY + player.height / 2.0f;
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
        vignetteIntensity = 1.0f;
        vignetteFade = 0.0f;
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

    }


    
    // === Movement Input ===
    if (currentInputMode == InputMode::Gamepad) {
        HandleGamepadInput(deltaTime);
    } else {
        HandleKeyboardInput(deltaTime);
    }
}



void DrawPlayer(const Player& player, Camera& camera) {
    DrawCapsule(player.position, Vector3 {player.position.x, player.height, player.position.z}, 10, 4, 4, RED);

    if (controlPlayer) weapon.Draw(camera); 


}

