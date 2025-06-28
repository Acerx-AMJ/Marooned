#include "level.h"
#include "dungeonGeneration.h"
#include <vector>

DungeonEntrance entranceToDungeon1 = {
    {0, 200, 0}, // position
    2, // linkedLevelIndex
};


//Player Position: X=-5653.32 Y=289.804 Z=6073.24
DungeonEntrance entranceToDungeon3 = {
    {-5653, 150, 6073}, 
    4,
};

//Player Position: X=6294.27 Y=299.216 Z=5515.47
DungeonEntrance entranceToDungeon4 = {
    {6294, 150, 5515},
    5, 
};


std::vector<LevelData> levels = {
    {
        "Middle Island", //display name
        "assets/MiddleIsland.png", //heightmap
        "",//dungeon path
        {0.0f, 300.0f, 0.0f}, //starting position
        -90.0f, //starting player rotation
        {0, 0, 0}, //raptor spawn center
        5, //raptor count
        false, //isDungeon
        {entranceToDungeon1, entranceToDungeon3, entranceToDungeon4}, //add entrance struct to level's vector of entrances. 
        0, //current level
        2, //next level, index 2

       
    },
    {
        "EyeballIsle", //place holder
        "assets/EyeballIsle.png",
        "",
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
        "assets/blank.png", //big blank heightmap incase we want water underneath the dungeon. 
        "assets/maps/map4.png",
        {0.0f, 300.0f, 0.0f}, //overwritten by green pixel 
        -90.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {}, //dungeons don't have level entrances
        2, 
        3, //next level index 3
        
       

    },
    {
        "Dungeon2",
        "assets/blank.png",
        "assets/maps/map6.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        3, 
        4,
       

    },

    {
        "Dungeon3",
        "assets/blank.png",
        "assets/maps/map7.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        4, 
        0,
    },
    {
        "Dungeon4",
        "assets/blank.png",
        "assets/maps/map8.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        5, 
        6,
    },
        {
        "Dungeon5",
        "assets/blank.png",
        "assets/maps/map9.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        6, 
        7,
    }
};

