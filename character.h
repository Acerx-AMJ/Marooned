#pragma once

#include "raylib.h"
#include "player.h"
#include <vector>

enum class CharacterType {
    Raptor,
    Skeleton,
    // Add more later: Zombie, Goblin, etc.
};


enum class DinoState {
    Idle,
    Chase,
    Attack,
    RunAway,
    Stagger,
    Reposition,
    Patrol,
    Death
};



class Character {
public:
    Vector3 position;
    Texture2D* texture;
    Vector3 previousPosition;
    DinoState state = DinoState::Idle;
    int frameWidth, frameHeight;
    int currentFrame, maxFrames;
    int rowIndex;
    float animationTimer, animationSpeed;
    float scale;
    float rotationY = 0.0f; // in degrees
    float stateTimer = 0.0f;
    bool isDead = false;
    float deathTimer = 0.0f;
    float attackCooldown = 0.0f;
    float chaseDuration = 0.0f;
    float hitTimer = 0.0f;
    float runawayAngleOffset = 0.0f;
    bool hasRunawayAngle = false;
    float idleThreshold = 0.0f; // for random movements when idle
    float randomDistance = 0.0f; //how far away to run before stopping. 
    float randomTime = 0.0f;
    float pathCooldownTimer = 0.0f;
    Vector2 lastPlayerTile = {-1, -1}; // Initialized to invalid tile
    float skeleSpeed = 650;
    bool playerVisible = false;
    float timeSinceLastSeen = 9999.0f;  // Large to start
    float forgetTime = 10.0f;           // After 3 seconds of no visibility, give up
    float radius = 50;
    BoundingBox collider;
    int maxHealth = 150;
    int currentHealth = maxHealth;
    CharacterType type;

    std::vector<Vector2> currentPath;
    std::vector<Vector3> currentWorldPath;


    Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl, int row = 0, CharacterType t = CharacterType::Raptor);
    BoundingBox GetBoundingBox() const;
    void Update(float deltaTime, Player& player, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors);
    Vector3 ComputeRepulsionForce(const std::vector<Character*>& allRaptors, float repulsionRadius = 500.0f, float repulsionStrength = 6000.0f);
    void UpdateRaptorAI(float deltaTime, Player& player, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors);
    void UpdateAI(float deltaTime, Player& player, Image heightmap, Vector3 terrainScale, const std::vector<Character*>& allRaptors); 
    void UpdateSkeletonAI(float deltaTime, Player& player, const std::vector<Character*>& allRaptors);
    void UpdateCollider(); // declare update method

    void eraseCharacters();
    void setPath();
    void TakeDamage(int amount);
    void Draw(Camera3D camera);
    void SetAnimation(int row, int frames, float speed);
    void playRaptorSounds();
};

