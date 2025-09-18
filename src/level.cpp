#include "level.h"
#include "dungeonGeneration.h"
#include <vector>

std::vector<PropSpawn> overworldProps;

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
        "assets/heightmaps/MiddleIsland.png", //heightmap
        "",//dungeon path
        {0.0f, 300.0f, 0.0f}, //starting position
        -90.0f, //starting player rotation
        {0, 0, 0}, //raptor spawn center
        5, //raptor count
        false, //isDungeon
        {entranceToDungeon1, entranceToDungeon3, entranceToDungeon4}, //add entrance struct to level's vector of entrances. 
        0, //current level
        2, //next level, index 2
        {
            { PropType::FirePit,  5200.f, -5600.f,  0.f, 100.0f }, // camp fire
            { PropType::Barrel,   5020.f, -5930.f, 30.f, 350.0f },
            { PropType::Barrel,   5100.f, -5810.f, -15.f, 100.0f },
        
        },
       
    },
    {
        "EyeballIsle", //place holder
        "assets/heightmaps/EyeballIsle.png",
        "",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0, 0.0f},
        0,
        false,
        {},
        1, 
        -1,
        {} 
   
    },
    {
        "Dungeon1",
        "assets/heightmaps/blank.png", //big blank heightmap incase we want water underneath the dungeon. 
        "assets/maps/map4.png",
        {0.0f, 300.0f, 0.0f}, //overwritten by green pixel 
        -90.0f, //starting look direction
        {0.0f, 0.0f, 0.0f}, //raptor spawn center
        0, //raptor count
        true, //isDungeon is true
        {}, //dungeons don't have level entrances
        2, //current level index
        3, //next level index 
        {} //outdoor props

    },
    {
        "Dungeon2",
        "assets/heightmaps/blank.png",
        "assets/maps/map6.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        3, 
        4,
        {}
       

    },

    {
        "Dungeon3",
        "assets/heightmaps/blank.png",
        "assets/maps/map7.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        4, 
        0,
        {}
    },
    {
        "Dungeon4",
        "assets/heightmaps/blank.png",
        "assets/maps/map8.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        5, 
        6,
        {}
    },
        {
        "Dungeon5",
        "assets/heightmaps/blank.png",
        "assets/maps/map9.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        6, 
        7,
        {}
    },
        {
        "Dungeon6",
        "assets/heightmaps/blank.png",
        "assets/maps/map10.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        7, 
        8,
        {}
    },
        {
        "Dungeon7",
        "assets/heightmaps/blank.png",
        "assets/maps/map11.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        8, 
        9,
        {}
    },
        {
        "Dungeon8",
        "assets/heightmaps/blank.png",
        "assets/maps/map12.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        9, 
        10,
        {}
    },
        {
        "Dungeon9",
        "assets/heightmaps/blank.png",
        "assets/maps/map14.png",
        {0.0f, 300.0f, 0.0f},
        180.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        10, 
        11,
        {}
    },
        {
        "Dungeon10",
        "assets/heightmaps/blank.png",
        "assets/maps/map15.png",
        {0.0f, 300.0f, 0.0f},
        -90.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        11, 
        12,
        {}
    },
        {
        "Dungeon11",
        "assets/heightmaps/blank.png",
        "assets/maps/map16.png",
        {0.0f, 300.0f, 0.0f},
        -90.0f,
        {0.0f, 0.0f, 0.0f},
        0, 
        true, //isDungeon is true
        {},
        12, 
        13,
        {}
    },
    
};

