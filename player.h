#pragma once

#include "raylib.h"

struct Player {
    Vector3 position;
    Vector3 velocity;
    Vector2 rotation;
    Vector3 forward;
    float speed = 500.0f;
    float gravity = 90.8f;
    float height = 150.0f;
    bool grounded = false;
};

// Initializes the player at a given position
void InitPlayer(Player& player, Vector3 startPosition);

// Updates player movement and physics
void UpdatePlayer(Player& player, float deltaTime, Mesh terrainMesh);

// Draws the player model or debug marker
void DrawPlayer(const Player& player);

float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale);
