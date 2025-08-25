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
        FireFireball(player.position, targetPoint, 5000, 10, false, true);
    }
    //give all weapons
    if (IsKeyPressed(KEY_SEMICOLON)) {
        if (player.collectedWeapons.size() < 3) {

            player.collectedWeapons.push_back(WeaponType::MagicStaff);
            
            
            player.activeWeapon = WeaponType::Blunderbuss;
        }

    }
}

void HandleMouseLook(float deltaTime){
    Vector2 mouseDelta = GetMouseDelta();
    float mouseSensitivity = 0.05f;
    player.rotation.y -= mouseDelta.x * mouseSensitivity;
    player.rotation.x -= mouseDelta.y * mouseSensitivity;
    player.rotation.x = Clamp(player.rotation.x, -30.0f, 30.0f);
    
}



