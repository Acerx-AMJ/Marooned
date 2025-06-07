#pragma once

#include "raylib.h"
#include "weapon.h"

struct Player {
    Vector3 position;
    Vector3 velocity;
    Vector2 rotation;
    Vector3 forward;
    Vector3 startPosition;
    Weapon weapon;
    
    bool running = false;
    float runSpeed = 800.0f; // faster than walk speed
    float walkSpeed = 500.0f; // regular speed

    float gravity = 980.0f;
    float height = 200.0f;
    float jumpStrength = 600; 
    float footstepTimer = 0.0;
    float maxHealth = 100;
    float currentHealth = maxHealth;
    float radius = 100;

    bool dying = false;
    bool dead = false;
    float deathTimer = 0.0f;


    bool grounded = false;
    bool isSwimming = false;
    bool isMoving = false;
    bool onBoard = false;
    bool disableMovement = false;

    float stamina = 100.0f;
    float maxStamina = 100.0f;
    bool canRun = true;
    bool canMove = true;

    void PlayFootstepSound();
    void TakeDamage(int amount);
    
};

// Initializes the player at a given position
void InitPlayer(Player& player, Vector3 startPosition);

// Updates player movement and physics
void UpdatePlayer(Player& player, float deltaTime, Mesh terrainMesh, Camera& camera);

void DrawPlayer(const Player& player, Camera& camera);
void HandleGamepadInput(float deltaTime);
void HandleKeyboardInput(float deltaTime);
void HandleMouseLook(float deltaTime);
void HandleStickLook(float deltaTime);
float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale);
