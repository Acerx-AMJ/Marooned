#pragma once

#include <string>
#include <vector>
#include "raylib.h"
#include <optional>

// Struct that holds all necessary data for a level
struct LevelData {
    std::string name;                 // Display name
    std::string heightmapPath;       // Path to the heightmap image
    Vector3 terrainScale;            // Scale of the terrain mesh
    Vector3 startPosition;           // Player starting position
    Vector3 raptorSpawnCenter;       // Center point for raptor spawns
    int raptorCount;                 // Number of raptors to spawn
    bool isDungeon;

    std::optional<BoundingBox> dungeonEntrance; // not every level has a dungeonEntrance
    
};

// All available levels
extern std::vector<LevelData> levels;


