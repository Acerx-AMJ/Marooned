#include "input.h"
#include "raymath.h"
#include "world.h"
#include <iostream>
#include "player.h"
#include "utilities.h"
#include "lighting.h"
#include "resourceManager.h"
InputMode currentInputMode = InputMode::KeyboardMouse;


void debugControls(Camera& camera, float deltaTime){

    if (IsKeyPressed(KEY_GRAVE)){
        debugInfo = !debugInfo;
    }

    if (IsKeyPressed(KEY_L)) {
        std::cout << "Player Position: ";
        DebugPrintVector(player.position);
        // isLoadingLevel = true;
        // InitDynamicLightmap(dungeonWidth * 4);
        // R.SetLightingShaderValues();
        // BuildStaticLightmapOnce(dungeonLights);
        // BuildDynamicLightmapFromFrameLights(frameLights);
        // isLoadingLevel = false;
        // LogDynamicLightmapNonBlack("testing: ");
        
    
    }

    if (debugInfo){
        //control player with arrow keys while in free cam. 
        Vector3 input = {0};
        if (IsKeyDown(KEY_UP)) input.z += 1;
        if (IsKeyDown(KEY_DOWN)) input.z -= 1;
        if (IsKeyDown(KEY_LEFT)) input.x += 1;
        if (IsKeyDown(KEY_RIGHT)) input.x -= 1;

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

    }

    //debug fireball
    if (IsKeyPressed(KEY_F) && debugInfo) {
        Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 targetPoint = Vector3Add(camera.position, Vector3Scale(camForward, 1000.0f));
        FireFireball(player.position, targetPoint, 5000, 10, false, true);
    }
    //give all weapons
    if (IsKeyPressed(KEY_SEMICOLON && debugInfo)) {
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



