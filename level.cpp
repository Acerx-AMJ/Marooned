#include "level.h"
#include "dungeonGeneration.h"
#include <vector>

DungeonEntrance entranceToDungeon1 = {
    {0, 200, 0},                             // position
    {{5300, 200, -6100}, {5500, 500, -5900}},       // bounding box
    2                                               // linkedLevelIndex
};

std::vector<LevelData> levels = {
    {
        "Middle Island", //display name
        "assets/MiddleIsland.png", //heightmap
        {16000.0f, 200.0f, 16000.0f}, //terrain scale
        {5475.0f, 300.0f, -5665.0f}, //starting position
        {0, 0, 0}, //raptor spawn center
        10, //raptor count
        false, //isDungeon
        {entranceToDungeon1}
       
    },
    {
        "EyeballIsle",
        "assets/EyeballIsle.png",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        {0.0f, 0, 0.0f},
        0,
        false, 
   
    },
    {
        "Dungeon1",
        "assets/blank.png",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
       

    }
};
