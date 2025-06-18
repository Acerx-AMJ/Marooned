#pragma once

#include <string>
#include <vector>
#include "raylib.h"
#include <optional>

struct DungeonEntrance {
    Vector3 position;
    int linkedLevelIndex;
    bool isTriggered = false;


};

// Struct that holds all necessary data for a level
struct LevelData {
    std::string name;                 // Display name
    std::string heightmapPath;       // Path to the heightmap image
    std::string dungeonPath;
    Vector3 terrainScale;            // Scale of the terrain mesh
    Vector3 startPosition;           // Player starting position
    float startingRotationY; // ← new
    Vector3 raptorSpawnCenter;       // Center point for raptor spawns
    int raptorCount;                 // Number of raptors to spawn
    bool isDungeon;

    std::vector<DungeonEntrance> entrances;
    
    
};

// All available levels
extern std::vector<LevelData> levels;


