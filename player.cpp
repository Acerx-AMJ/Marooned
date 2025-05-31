#include "player.h"
#include "raymath.h"
#include "world.h"
#include <iostream>
#include "resources.h"
#include "input.h"
#include "boat.h"

void InitPlayer(Player& player, Vector3 startPosition) {
    player.position = startPosition;
    player.velocity = {0, 0, 0};
    player.grounded = false;
    
}

void HandleMouseLook(float deltaTime){
    Vector2 mouseDelta = GetMouseDelta();
    float mouseSensitivity = 0.03f;
    player.rotation.y -= mouseDelta.x * mouseSensitivity;
    player.rotation.x -= mouseDelta.y * mouseSensitivity;
    player.rotation.x = Clamp(player.rotation.x, -89.0f, 89.0f);

}

void HandleKeyboardInput(float deltaTime) {

    Vector3 input = {0};
    if (IsKeyDown(KEY_W)) input.z += 1;
    if (IsKeyDown(KEY_S)) input.z -= 1;
    if (IsKeyDown(KEY_A)) input.x += 1;
    if (IsKeyDown(KEY_D)) input.x -= 1;

    player.running = IsKeyDown(KEY_LEFT_SHIFT);
    float speed = player.running ? player.runSpeed : player.walkSpeed;

    if (input.x != 0 || input.z != 0) {
        input = Vector3Normalize(input);
        float yawRad = DEG2RAD * player.rotation.y;

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
    float moveSensitivity = 1.0f;

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

void UpdatePlayer(Player& player, float deltaTime, Mesh terrainMesh) {
    // --- Boarding Check ---
    if (!player.onBoard) {
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

    if (feetY <= groundY) {
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

    // === Movement Input ===
    if (currentInputMode == InputMode::Gamepad) {
        HandleGamepadInput(deltaTime);
    } else {
        HandleKeyboardInput(deltaTime);
    }
}



void DrawPlayer(const Player& player) {
    DrawCapsule(player.position, Vector3 {player.position.x, player.height, player.position.z}, 10, 4, 4, RED);
}

