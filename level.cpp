#include "level.h"
#include "dungeonGeneration.h"
#include <vector>

DungeonEntrance entranceToDungeon1 = {
    {0, 200, 0}, // position
    2, // linkedLevelIndex
};

DungeonEntrance exitDungeon1 = {
    {0, 0, 0}, // position
    0, // linkedLevelIndex
};



std::vector<LevelData> levels = {
    {
        "Middle Island", //display name
        "assets/MiddleIsland.png", //heightmap
        "",
        {16000.0f, 200.0f, 16000.0f}, //terrain scale
        {0.0f, 300.0f, 0.0f}, //starting position
        -90.0f,
        {0, 0, 0}, //raptor spawn center
        5, //raptor count
        false, //isDungeon
        {entranceToDungeon1} //add entrance struct to level's vector of entrances. 
       
    },
    {
        "EyeballIsle",
        "assets/EyeballIsle.png",
        "",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0, 0.0f},
        0,
        false, 
   
    },
    {
        "Dungeon1",
        "assets/blank.png",
        "assets/maps/map4.png",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        -90.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {exitDungeon1}
        
       

    },
    {
        "Dungeon2",
        "assets/blank.png",
        "assets/maps/map6.png",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
       

    }
};
