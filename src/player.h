#pragma once

#include "raylib.h"
#include "weapon.h"
#include "inventory.h"

extern Weapon weapon;
extern MeleeWeapon meleeWeapon;
extern MagicStaff magicStaff;



struct Player {
    Vector3 position;
    Vector3 velocity;
    Vector2 rotation;
    Vector3 forward;
    Vector3 startPosition;

    BoundingBox meleeHitbox;
    BoundingBox blockHitbox;

    Inventory inventory;

    int gold = 0;
    float displayedGold = 0.0f;

    bool running = false;
    float runSpeed = 800.0f; // faster than walk speed
    float walkSpeed = 500.0f; // regular speed
    float startingRotationY = 0.0f; // in degrees

    float gravity = 980.0f;
    float height = 200.0f;
    float jumpStrength = 600.0f; 
    float footstepTimer = 0.0f;
    float swimTimer = 0.0f;
    float maxHealth = 100.0f;
    float currentHealth = maxHealth;
    float maxMana = 100.0f;
    float currentMana = maxMana;
    float radius = 150.0f;
    float hitTimer = 0.0f;
    bool dying = false;
    bool dead = false;
    float deathTimer = 0.0f;
    float targetFOV = 50.0f;  //change camera.fovy slightly on hit
    float groundY;
    bool grounded = false;
    bool isSwimming = false;
    bool overLava = false;
    bool isMoving = false;
    bool onBoard = false;
    bool disableMovement = false;
    bool blocking = false;
    

    float stamina = 100.0f;
    float maxStamina = 100.0f;
    bool canRun = true;
    bool canMove = true;

    std::vector<WeaponType> collectedWeapons;

    int currentWeaponIndex = -1; // 


    WeaponType activeWeapon = WeaponType::None;

    float switchTimer = 0.0f;
    float switchDuration = 0.3f; // time to lower or raise
    BoundingBox GetBoundingBox() const;
    void PlayFootstepSound();
    void TakeDamage(int amount);
    void EquipNextWeapon();
};



// Initializes the player at a given position
void InitPlayer(Player& player, Vector3 startPosition);

// Updates player movement and physics
void UpdatePlayer(Player& player, float deltaTime, Camera& camera);

void DrawPlayer(const Player& player, Camera& camera);
void InitSword(MeleeWeapon& sword);
void InitBlunderbuss(Weapon& blunderbuss);
void InitMagicStaff(MagicStaff& magicStaff);
