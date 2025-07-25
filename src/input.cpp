#include "input.h"
#include "raymath.h"
#include "vegetation.h"
#include "world.h"
#include <iostream>
#include "player.h"
#include "dungeonGeneration.h"
#include "pathfinding.h"
#include "sound_manager.h"

InputMode currentInputMode = InputMode::KeyboardMouse;



void UpdateCameraWithGamepad(Camera3D& camera) {
    if (!IsGamepadAvailable(0)) return;

    float lookSensitivity = 0.03f;

    float yawInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    float pitchInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

    float yaw = -yawInput * lookSensitivity;
    float pitch = -pitchInput * lookSensitivity;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    // Apply yaw (rotate around up)
    Matrix yawRot = MatrixRotate(camera.up, yaw);
    forward = Vector3Transform(forward, yawRot);

    // Apply pitch (rotate around right)
    Matrix pitchRot = MatrixRotate(right, pitch);
    forward = Vector3Transform(forward, pitchRot);

    // Move camera position
    float moveSpeed = 1.0f;
    Vector2 stick = {
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)
    };

    camera.position = Vector3Add(camera.position, Vector3Scale(right, stick.x * moveSpeed));
    camera.position = Vector3Add(camera.position, Vector3Scale(forward, -stick.y * moveSpeed));
    camera.target = Vector3Add(camera.position, forward);
}

void debugControls(Camera& camera){

    if (IsKeyPressed(KEY_GRAVE)){
        debugInfo = !debugInfo;
    }

    if (IsKeyPressed(KEY_L)) {
        std::cout << "Player Position: X=" << player.position.x
                << " Y=" << player.position.y
                << " Z=" << player.position.z << "\n";
    }

    if (IsKeyPressed(KEY_I)){
        RemoveAllVegetation();
    }
    if (IsKeyPressed(KEY_O)){
        //open all doors //what if a door is locked and open?
        for (DoorwayInstance& d : doorways){
            d.isOpen = true;
        }

        for (Door& door : doors){
            door.isOpen = true;
        }
    }

    if (IsKeyPressed(KEY_P)) {
        //close all doors
        for (DoorwayInstance& d : doorways){
            d.isOpen = false;
        }

        for (Door& door : doors){
            door.isOpen = false;
        }
        
    }
    //debug fireball
    if (IsKeyPressed(KEY_F)) {
        Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 targetPoint = Vector3Add(camera.position, Vector3Scale(camForward, 1000.0f));
        FireFireball(player.position, targetPoint, 5000, 10, false, true);
    }
    //give all weapons
    if (IsKeyPressed(KEY_SEMICOLON)) {
        if (player.collectedWeapons.size() == 0) {
            player.collectedWeapons.push_back(WeaponType::Blunderbuss);
            player.collectedWeapons.push_back(WeaponType::Sword);
            player.collectedWeapons.push_back(WeaponType::MagicStaff);
            
            
            player.activeWeapon = WeaponType::Blunderbuss;
        }

    }





}


void UpdateInputMode() {

    // Check gamepad activity
    if (IsGamepadAvailable(0)) {
        float lx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float ly = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        float rx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        float ry = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

        if (fabsf(lx) > 0.1f || fabsf(ly) > 0.1f || fabsf(rx) > 0.1f || fabsf(ry) > 0.1f) {
            currentInputMode = InputMode::Gamepad;
            return;
        }

        for (int b = 0; b <= GAMEPAD_BUTTON_MIDDLE_RIGHT; b++) {
            if (IsGamepadButtonPressed(0, b)) {
                currentInputMode = InputMode::Gamepad;
                return;
            }
        }
    }

    // Check mouse/keyboard activity
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) ||
        IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ||
        GetMouseDelta().x != 0 || GetMouseDelta().y != 0) {
        currentInputMode = InputMode::KeyboardMouse;
    }
}

void HandleMouseLook(float deltaTime){
    Vector2 mouseDelta = GetMouseDelta();
    float mouseSensitivity = 0.05f;
    player.rotation.y -= mouseDelta.x * mouseSensitivity;
    player.rotation.x -= mouseDelta.y * mouseSensitivity;
    player.rotation.x = Clamp(player.rotation.x, -89.0f, 89.0f);

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

