#pragma once

#include "raylib.h"

enum class WeaponType { 
    Blunderbuss, 
    Sword 
};



struct MeleeWeapon {
    Model model;
    Vector3 scale = { 1.0f, 1.0f, 1.0f };

    float swingTimer = 0.0f;
    float swingDuration = 0.7f;
    bool swinging = false;

    bool hitboxActive = false;
    float hitboxTimer = 0.0f;
    float hitboxDuration = 0.25f; // how long hitbox stays active

    float hitWindowStart = 0.1f;  // seconds into the swing
    float hitWindowEnd = 0.25f;   // seconds into the swing
    bool hitboxTriggered = false;

    bool blocking = false;
    float blockLerp = 0.0f; // 0 → 1 smooth transition
    float blockSpeed = 6.0f; // how fast it moves to blocking position

    // Block pose offsets
    float blockForwardOffset = 50.0f;
    float blockVerticalOffset = -30.0f;
    float blockSideOffset = 0.0f;



    float swingAmount = 20.0f;   // how far forward it jabs or sweeps
    float swingOffset = 0.0f;    // forward movement

    float verticalSwingOffset = -30.0f;
    float verticalSwingAmount = 60.0f; // how far it chops down

    float horizontalSwingOffset = 0.0f;
    float horizontalSwingAmount = 20.0f; // little lateral arc


    float cooldown = 1.0f;
    float timeSinceLastSwing = 999.0f;

    float forwardOffset = 60.0f;
    float sideOffset = 20.0f; // pull it left of the screen
    float verticalOffset = -40.0f;
    void StartBlock();
    void EndBlock();
    void StartSwing();
    void PlaySwipe();
    void Update(float deltaTime);
    void Draw(const Camera& camera);
};


struct Weapon {
    Model model;
    Vector3 scale = { 1.0f, 1.0f, 1.0f };
    Vector3 muzzlePos;
    float flashSize = 120.0f;
    float forwardOffset = 80.0f;
    float sideOffset = 30.0f;
    float verticalOffset = -30.0f;

    float recoil = 0.0f;
    float recoilAmount = 15.0f;
    float recoilRecoverySpeed = 30.0f;

    float lastFired = -999.0f;
    float fireCooldown; //set in player init

    

    Texture2D muzzleFlashTexture;
    float flashDuration = 0.1f;
    float flashTimer = 0.0f;
    //delayed reload sound
    bool reloadScheduled = false;
    float reloadTimer = 0.0f;
    float reloadDelay = 1.6f; // adjust as needed
    float reloadDip = 0.0f; // controls how far the gun dips down
 

    void Fire(Camera& camera);
    void Update(float deltaTime);
    void Draw(const Camera& camera);
};

