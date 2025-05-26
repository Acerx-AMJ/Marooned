#include "player.h"
#include "raymath.h"
#include "world.h"
#include <iostream>

void InitPlayer(Player& player, Vector3 startPosition) {
    player.position = startPosition;
    player.velocity = {0, 0, 0};
    player.grounded = false;
    
}

void UpdatePlayer(Player& player, float deltaTime, Mesh terrainMesh) {
    // Apply gravity
    if (!player.grounded) {
        player.velocity.y -= player.gravity * deltaTime;
    }

    player.running = IsKeyDown(KEY_LEFT_SHIFT);

    float currentSpeed = player.running ? player.runSpeed : player.walkSpeed;




    Vector2 mouseDelta = GetMouseDelta();
    float mouseSensitivity = 0.03f;
    player.rotation.y -= mouseDelta.x * mouseSensitivity; // Yaw (turn left/right)
    player.rotation.x -= mouseDelta.y * mouseSensitivity; // Pitch (look up/down)

    player.rotation.x = Clamp(player.rotation.x, -89.0f, 89.0f);

    // Simple movement (WASD)
    Vector3 input = {0};
    if (IsKeyDown(KEY_W)) input.z += 1;
    if (IsKeyDown(KEY_S)) input.z -= 1;
    if (IsKeyDown(KEY_A)) input.x += 1;
    if (IsKeyDown(KEY_D)) input.x -= 1;


    if (input.x != 0 || input.z != 0) {
        input = Vector3Normalize(input);

        // Convert yaw from degrees to radians
        float yawRad = DEG2RAD * player.rotation.y;

        // Calculate forward and right vectors based on yaw
        Vector3 forward = { sinf(yawRad), 0, cosf(yawRad) };
        Vector3 right = { forward.z, 0, -forward.x }; // Perpendicular to forward

        // Combine input direction with facing
        Vector3 moveDir = {
            forward.x * input.z + right.x * input.x,
            0,
            forward.z * input.z + right.z * input.x
        };

        moveDir = Vector3Scale(Vector3Normalize(moveDir), currentSpeed * deltaTime);

        player.position.x += moveDir.x;
        player.position.z += moveDir.z;

        // Optional: Store forward direction
        player.forward = forward;
    }



    // Check ground height
    float groundY = GetHeightAtWorldPosition(player.position, heightmap, terrainScale);
    float feetY = player.position.y- player.height / 2.0f;

    if (feetY <= groundY) {
        player.grounded = true;
        player.velocity.y = 0;
        player.position.y = groundY + player.height / 2.0f;
    } else {
        player.grounded = false;
        //player.position.y += player.velocity.y * deltaTime;
        float targetY = groundY + player.height / 2.0f;
        player.position.y = Lerp(player.position.y, targetY, 0.5f * deltaTime);
    }

    if (player.grounded && IsKeyPressed(KEY_SPACE)) {
        player.velocity.y = player.jumpStrength;
        player.grounded = false;
    }

    // apply vertical velocity after jumping
    if (!player.grounded) {
        player.velocity.y -= player.gravity * deltaTime;
        player.position.y += player.velocity.y * deltaTime;
    }




}

void DrawPlayer(const Player& player) {
    DrawCapsule(player.position, Vector3 {player.position.x, player.height, player.position.z}, 40, 4, 4, RED);
}

float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale) {
    int width = heightmap.width;
    int height = heightmap.height;
    unsigned char* pixels = (unsigned char*)heightmap.data;

    // Convert world X/Z into heightmap image coordinates
    float xPercent = (position.x + terrainScale.x / 2.0f) / terrainScale.x;
    float zPercent = (position.z + terrainScale.z / 2.0f) / terrainScale.z;

    // Clamp to valid range
    xPercent = Clamp(xPercent, 0.0f, 1.0f);
    zPercent = Clamp(zPercent, 0.0f, 1.0f);

    // Convert to pixel indices
    int x = (int)(xPercent * (width - 1));
    int z = (int)(zPercent * (height - 1));
    int index = z * width + x;

    // Get grayscale pixel and scale to world height
    float heightValue = (float)pixels[index] / 255.0f;
    return heightValue * terrainScale.y;
}
