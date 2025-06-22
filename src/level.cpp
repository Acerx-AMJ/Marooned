#include "level.h"
#include "dungeonGeneration.h"
#include <vector>

DungeonEntrance entranceToDungeon1 = {
    {0, 200, 0}, // position
    2, // linkedLevelIndex
};

DungeonEntrance exitDungeon1 = {
    {0, 0, 0}, // position, location of the entrance to dungeon
    0, // linkedLevelIndex
};



std::vector<LevelData> levels = {
    {
        "Middle Island", //display name
        "assets/MiddleIsland.png", //heightmap
        "",
        {16000.0f, 200.0f, 16000.0f}, //terrain scale
        {0.0f, 300.0f, 0.0f}, //starting position
        -90.0f, //starting player rotation
        {0, 0, 0}, //raptor spawn center
        5, //raptor count
        false, //isDungeon
        {entranceToDungeon1}, //add entrance struct to level's vector of entrances. 
        0, //current level
        2, //next level, index 2

       
    },
    {
        "EyeballIsle", //place holder
        "assets/EyeballIsle.png",
        "",
        {16000.0f, 300.0f, 16000.0f},
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0, 0.0f},
        0,
        false,
        {},
        1, 
        -1, 
   
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
        {exitDungeon1},
        2, 
        3, //next level index 3
        
       

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
        {},
        3, 
        4,
       

    }
};
