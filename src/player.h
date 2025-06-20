#pragma once

#include "raylib.h"
#include "weapon.h"
#include "inventory.h"


struct Player {
    Vector3 position;
    Vector3 velocity;
    Vector2 rotation;
    Vector3 forward;
    Vector3 startPosition;
    Weapon weapon;
    MeleeWeapon meleeWeapon;
    BoundingBox meleeHitbox;
    BoundingBox blockHitbox;

    Inventory inventory;

    bool running = false;
    float runSpeed = 800.0f; // faster than walk speed
    float walkSpeed = 500.0f; // regular speed
    float startingRotationY = 0.0f; // in degrees

    float gravity = 980.0f;
    float height = 200.0f;
    float jumpStrength = 600; 
    float footstepTimer = 0.0;
    float maxHealth = 100;
    float currentHealth = maxHealth;
    float radius = 150;

    bool dying = false;
    bool dead = false;
    float deathTimer = 0.0f;

    float groundY;
    bool grounded = false;
    bool isSwimming = false;
    bool isMoving = false;
    bool onBoard = false;
    bool disableMovement = false;
    bool blocking = false;

    float stamina = 100.0f;
    float maxStamina = 100.0f;
    bool canRun = true;
    bool canMove = true;

    //WeaponType pendingWeapon; // store the one we're switching to
    //WeaponSwitchState switchState = WeaponSwitchState::Idle;
    WeaponType activeWeapon = WeaponType::Blunderbuss;

    float switchTimer = 0.0f;
    float switchDuration = 0.3f; // time to lower or raise
    BoundingBox GetBoundingBox() const;
    void PlayFootstepSound();
    void TakeDamage(int amount);



    //void PlaySwipe();
};

// Initializes the player at a given position
void InitPlayer(Player& player, Vector3 startPosition);

// Updates player movement and physics
void UpdatePlayer(Player& player, float deltaTime, Mesh& terrainMesh, Camera& camera);

void DrawPlayer(const Player& player, Camera& camera);
void HandleGamepadInput(float deltaTime);
void HandleKeyboardInput(float deltaTime);
void HandleMouseLook(float deltaTime);
void HandleStickLook(float deltaTime);
//float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale);
