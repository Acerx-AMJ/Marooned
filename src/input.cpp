#include "input.h"
#include "raymath.h"
#include "world.h"
#include <iostream>
#include "player.h"



#include "utilities.h"

InputMode currentInputMode = InputMode::KeyboardMouse;


void debugControls(Camera& camera){

    if (IsKeyPressed(KEY_GRAVE)){
        debugInfo = !debugInfo;
    }

    if (IsKeyPressed(KEY_L)) {
        std::cout << "Player Position: ";
        DebugPrintVector(player.position);
    }



    //debug fireball
    if (IsKeyPressed(KEY_F)) {
        Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 targetPoint = Vector3Add(camera.position, Vector3Scale(camForward, 1000.0f));
        FireFireball(player.position, targetPoint, 5000, 10, false);
    }
    //give all weapons
    if (IsKeyPressed(KEY_SEMICOLON)) {
        if (player.collectedWeapons.size() < 3) {

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
    //maybe this should be  inside player. 
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


