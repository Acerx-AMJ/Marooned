#include "level.h"

std::vector<LevelData> levels = {
    {
        "Middle Island", //display name
        "assets/MiddleIsland.png", //path
        {16000.0f, 200.0f, 16000.0f}, //terrain scale
        {5475.0f, 300.0f, -5665.0f}, //start position
        {0, 0, 0}, //raptor spawn center - 3000 radius
        10, //raptor count
        false, //isDungeon
        BoundingBox{ Vector3{-100, 0, 100}, Vector3{-100, 300, 100} } // ← Dungeon entrance
    },
    {
        "EyeballIsle",
        "assets/EyeballIsle.png",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        {0.0f, 0, 0.0f},
        0,
        false, 
        std::nullopt // ← no entrance
    },
    {
        "Dungeon1",
        "assets/blank.png",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        std::nullopt // ← no entrance

    }
};
