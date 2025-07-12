#include "dungeonGeneration.h"
#include "raymath.h"
#include "resources.h"
#include <iostream>
#include "player.h"
#include "vector"
#include "world.h"
#include "rlgl.h"
#include "sound_manager.h"
#include "transparentDraw.h"



std::vector<FloorTile> floorTiles;
std::vector<WallInstance> wallInstances;
std::vector<CeilingTile> ceilingTiles;
std::vector<BarrelInstance> barrelInstances;
std::vector<SpiderWebInstance> spiderWebs;
std::vector<ChestInstance> chestInstances;
std::vector<DoorwayInstance> doorways;
std::vector<Door> doors;

std::vector<PillarInstance> pillars;
std::vector<WallRun> wallRunColliders;
std::vector<LightSource> dungeonLights; //static lights. 
std::vector<LightSource> bulletLights; //lights attached to bullets for testing.
std::vector<Fire> fires;



Image dungeonImg; //save the dungeon info globaly
Color* dungeonPixels = nullptr;
int dungeonWidth = 0;
int dungeonHeight = 0;

// === Constants ===
const Color COLOR_BLACK  = { 0, 0, 0, 255 }; //walls
const Color COLOR_BLUE   = { 0, 0, 255, 255 }; //barrels
const Color COLOR_RED   = { 255, 0, 0, 255 }; //enemies


//Dungeon Legend

//   Transparent = Void (no floor tiles)

//⬛ Black = Wall

//⬜ White = Floor (the whole dungeon is filled with floor not just white pixels)

//🟩 Green = Player Start

//🔵 Blue = Barrel

//🟥 Red = Skeleton

//🟨 Yellow = Light

//🟪 Purple = Doorway (128, 0, 128)

//   Teal = Dungeon Exit (0, 128, 128)

//🟧 Orange = Next Level (255, 128, 0)

//   Aqua = locked door (0, 255, 255)

//   Magenta = Pirate (255, 0, 255)

//💗 Pink = health pot (255, 105, 180)

//⭐ Gold = key (255, 200, 0)

//  Sky Blue = Chest (0, 128, 255)

// ⚫ Dark Gray = Spider (64, 64, 64)

void InitChests() {
    //chestModel = LoadModel("assets/models/chest.glb");
    //chestAnimations = LoadModelAnimations("assets/models/chest.glb", &chestAnimCount);

}



void UpdateDungeonChests() {
    
    const int OPEN_START_FRAME = 0;
    const int OPEN_END_FRAME = 10;

    for (ChestInstance& chest : chestInstances) {
        float distToPlayer = Vector3Distance(player.position, chest.position);
        if (distToPlayer < 300 && IsKeyPressed(KEY_E) && !chest.open){
            chest.animPlaying = true;
            chest.animFrame = 0.0f;


            SoundManager::GetInstance().Play("chestOpen");
        }

        if (chest.animPlaying) {
            chest.animFrame += GetFrameTime() * 24.0f;

            if (chest.animFrame > OPEN_END_FRAME) {
                chest.animFrame = OPEN_END_FRAME;
                chest.animPlaying = false;
                chest.open = true;
            }

            UpdateModelAnimation(chest.model, chest.animations[0], (int)chest.animFrame);

            
        }
        else if (chest.open && chest.canDrop) { //wait until the animation is finished before dropping the item. 
            chest.canDrop = false;
            UpdateModelAnimation(chest.model, chest.animations[0], OPEN_END_FRAME);
            Vector3 pos = {chest.position.x, chest.position.y + 100, chest.position.z};
            Collectable key(CollectableType::Key, pos, &keyTexture, 100);
            
            collectables.push_back(key);
            
        }

    }

}


BoundingBox MakeWallBoundingBox(const Vector3& start, const Vector3& end, float thickness, float height) {
    Vector3 min = Vector3Min(start, end);
    Vector3 max = Vector3Max(start, end);
    

    // Expand by half thickness in perpendicular direction
    if (start.x == end.x) {
        // Vertical wall (same x, different z)
        min.x -= thickness * 0.5f;
        max.x += thickness * 0.5f;
    } else {
        // Horizontal wall (same z, different x)
        min.z -= thickness * 0.5f;
        max.z += thickness * 0.5f;
    }

    max.y += height + 400; // dont jump over walls.

    return { min, max };
}


void LoadDungeonLayout(const std::string& imagePath) {
    if (dungeonPixels) {
        UnloadImageColors(dungeonPixels); //erase the previous pixels 
    }

    dungeonImg = LoadImage(imagePath.c_str());
    dungeonPixels = LoadImageColors(dungeonImg);
    dungeonWidth = dungeonImg.width;
    dungeonHeight = dungeonImg.height;


}

Vector3 FindSpawnPoint(Color* pixels, int width, int height, float tileSize, float baseY) {
    //look for the green pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color current = pixels[y * width + x];
            if (current.r == 0 && current.g == 255 && current.b == 0) { //pure green
                return GetDungeonWorldPos(x, y, tileSize, baseY);
            }
        }
    }
    return Vector3{ 0, baseY, 0 }; // fallback if none found
}



void GenerateFloorTiles(float baseY) {
    floorTiles.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color pixel = GetImageColor(dungeonImg, x, y);

            // Only skip transparent pixels
            if (pixel.a == 0) continue;

            Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

            FloorTile tile;
            tile.position = pos;
            tile.tint = WHITE; // Or tint based on pixel.r/g/b if you want variety
            // int rn = GetRandomValue(1, 2);
            // switch (rn){
            //     case 1:tile.floorTile = floorTile; break; 
            //     case 2:tile.floorTile = floorTile2; break;
            //     ///case 3:tile.floorTile = floorTile3; break; I broke the other tile models in blender. 
            // }
            tile.floorTile = floorTile;


            floorTiles.push_back(tile);
        }
    }
}





// === Helper Functions ===
inline bool ColorsEqual(Color a, Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

inline bool IsWallColor(Color c) {
    return ColorsEqual(c, COLOR_BLACK);
}

inline bool IsBarrelColor(Color c) {
    return ColorsEqual(c, COLOR_BLUE);
}

inline bool IsEnemyColor(Color c) {
    return ColorsEqual(c, COLOR_RED);
}




void GenerateWallTiles(float baseY) {
    //and create bounding boxes
    wallInstances.clear();
    wallRunColliders.clear();

    float wallThickness = 50.0f;
    float wallHeight = 200.0f;

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (IsBarrelColor(current)) continue;
            if (!IsWallColor(current)) continue;

            // === Horizontal Pair ===
            if (x < dungeonWidth - 1) {
                Color right = dungeonPixels[y * dungeonWidth + (x + 1)];

                if (current.a == 0 || right.a == 0) continue; // skip if either tile is void

                if (IsWallColor(right) && !IsBarrelColor(right)) {
                    Vector3 a = GetDungeonWorldPos(x, y, tileSize, baseY);
                    Vector3 b = GetDungeonWorldPos(x + 1, y, tileSize, baseY);

                    // Wall positioned halfway between tiles
                    Vector3 mid = Vector3Lerp(a, b, 0.5f);
                    mid.y = baseY;

                    WallInstance wall;
                    wall.position = mid;
                    wall.rotationY = 90.0f;
                    wall.tint = WHITE;
                    wallInstances.push_back(wall);

                    // Move them down to match the wall visuals
                    a.y -= 190.0f;
                    b.y -= 190.0f;

                    BoundingBox bounds = MakeWallBoundingBox(a, b, wallThickness, wallHeight);

                    wallRunColliders.push_back({ a, b, 90.0f, bounds });
                }
            }

            // === Vertical Pair ===
            if (y < dungeonHeight - 1) {
                Color down = dungeonPixels[(y + 1) * dungeonWidth + x];

                if (current.a == 0 || down.a == 0) continue; // skip if either tile is void

                if (IsWallColor(down) && !IsBarrelColor(down)) {
                    Vector3 a = GetDungeonWorldPos(x, y, tileSize, baseY);
                    Vector3 b = GetDungeonWorldPos(x, y + 1, tileSize, baseY);

                    Vector3 mid = Vector3Lerp(a, b, 0.5f);
                    mid.y = baseY;

                    WallInstance wall;
                    wall.position = mid;
                    wall.rotationY = 0.0f;
                    wall.tint = WHITE;
                    wallInstances.push_back(wall);

                    a.y -= 190.0f;
                    b.y -= 190.0f;

                    BoundingBox bounds = MakeWallBoundingBox(a, b, wallThickness, wallHeight);

                    wallRunColliders.push_back({ a, b, 0.0f, bounds });
                }
            }
        }
    }
    std::cout << "walls.size() = " << wallInstances.size() << "\n";
    for (auto& w : wallInstances) printf("Wall at %f,%f,%f\n", w.position.x, w.position.y, w.position.z);
}

void GenerateSideColliders(Vector3 pos, float rotationY, DoorwayInstance& archway){
    // 1️⃣ Define local side offset in door-local space
    Vector3 localOffset = { 150, 0, 0 };

    // 2️⃣ Rotate local offset by (rotationY + 90°) so it's perpendicular to the door's opening
    float sideRotation = rotationY + 90.0f * DEG2RAD;
    float cosR = cosf(sideRotation);
    float sinR = sinf(sideRotation);

    Vector3 rotatedOffset = {
        localOffset.x * cosR - localOffset.z * sinR,
        0,
        localOffset.x * sinR + localOffset.z * cosR
    };

    // 3️⃣ Compute world positions for side colliders
    Vector3 leftPos = {
        pos.x - rotatedOffset.x,
        pos.y,
        pos.z - rotatedOffset.z
    };

    Vector3 rightPos = {
        pos.x + rotatedOffset.x,
        pos.y,
        pos.z + rotatedOffset.z
    };

    // 4️⃣ Dimensions for the side walls
    float sideWidth = 20.0f;
    float sideHeight = 400.0f;
    float sideDepth = 50.0f;

    // 5️⃣ Create bounding boxes
    BoundingBox leftBox = MakeDoorBoundingBox(
        leftPos,
        rotationY,
        sideWidth * 0.5f,
        sideHeight,
        sideDepth
    );

    BoundingBox rightBox = MakeDoorBoundingBox(
        rightPos,
        rotationY,
        sideWidth * 0.5f,
        sideHeight,
        sideDepth
    );

    // 6️⃣ Store them
    archway.sideColliders.push_back(leftBox);
    archway.sideColliders.push_back(rightBox);

}

void GenerateDoorways(float baseY, int currentLevelIndex) {
    doorways.clear();
    
    for (int y = 1; y < dungeonHeight - 1; y++) {
        for (int x = 1; x < dungeonWidth - 1; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            bool isDoor = (current.r == 128 && current.g == 0 && current.b == 128);   // purple
            bool isExit     = (current.r == 0 && current.g == 128 && current.b == 128);   // teal
            bool nextLevel = (current.r == 255 && current.g == 128 && current.b == 0); //orange
            bool lockedDoor = (current.r == 0 && current.g == 255 && current.b == 255); //Aqua

            if (!isDoor && !isExit && !nextLevel && !lockedDoor) continue;

            // Check surrounding walls to determine door orientation
            Color left = dungeonPixels[y * dungeonWidth + (x - 1)];
            Color right = dungeonPixels[y * dungeonWidth + (x + 1)];
            Color up = dungeonPixels[(y - 1) * dungeonWidth + x];
            Color down = dungeonPixels[(y + 1) * dungeonWidth + x];

            bool wallLeft = left.r == 0 && left.g == 0 && left.b == 0;
            bool wallRight = right.r == 0 && right.g == 0 && right.b == 0;
            bool wallUp = up.r == 0 && up.g == 0 && up.b == 0;
            bool wallDown = down.r == 0 && down.g == 0 && down.b == 0;

            float rotationY = 0.0f;
            if (wallLeft && wallRight) {
                rotationY = 90.0f * DEG2RAD;
            } else if (wallUp && wallDown) {
                rotationY = 0.0f;
            } else {
                continue; // not a valid doorway
            }

            Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);
            DoorwayInstance archway = { pos, rotationY, false, false, WHITE, x, y };

            GenerateSideColliders(pos, rotationY, archway);




            if (isExit) { //teal
                archway.linkedLevelIndex = previousLevelIndex; //go back outside. 
            }else if (nextLevel){ //orange
                archway.linkedLevelIndex = levels[currentLevelIndex].nextLevel; //door to next level
            }else if (lockedDoor){ //Aqua
                archway.isLocked = true; //locked door
            } else { //purple
                archway.linkedLevelIndex = -1; //regular door
            }
        
            doorways.push_back(archway);
        }
    }

    GenerateDoorsFromArchways();
}


void GenerateDoorsFromArchways() {
    doors.clear();

    for (const DoorwayInstance& dw : doorways) {
        if (dw.isOpen) continue; // skip if this archway should remain open

        // Match position/rotation of archway
        Door door;
        door.position = dw.position;
        door.rotationY = dw.rotationY + DEG2RAD * 90.0f;
        door.isOpen = false;
        door.isLocked = dw.isLocked;
        door.doorTexture = &doorTexture;
        door.scale = {300, 365, 1}; //stretch it taller
        door.tileX = dw.tileX;
        door.tileY = dw.tileY;
        door.sideColliders = dw.sideColliders; //side colliders for when door is open
        
        float halfWidth = 200.0f;   // Half of the 400-unit wide doorway
        float height = 365.0f;
        float depth = 20.0f;        // Thickness into the doorway (forward axis)

        door.collider = MakeDoorBoundingBox(door.position, door.rotationY, halfWidth, height, depth); //covers the whole archway

        if (dw.linkedLevelIndex == previousLevelIndex) {
            door.doorType = DoorType::ExitToPrevious;
        } else if (dw.linkedLevelIndex == levels[levelIndex].nextLevel) {
            door.doorType = DoorType::GoToNext;
            
        } else {
            door.doorType = DoorType::Normal;
}

        door.linkedLevelIndex = dw.linkedLevelIndex; //get the level index from the archway

        doors.push_back(door);

    }
}

bool IsDoorOpenAt(int x, int y) {
    for (const Door& door : doors) {
        if (door.tileX == x && door.tileY == y) {
            return door.isOpen;
        }
    }
    return true; // If no door is found, assume it's open (or not a real door)
}


void GenerateCeilingTiles(float ceilingOffsetY) {
    ceilingTiles.clear();
    //mirror the floor
    for (const FloorTile& floor : floorTiles) {
        CeilingTile ceiling;
        ceiling.position = Vector3Add(floor.position, {0, ceilingOffsetY, 0});
        ceiling.tint = GRAY; // default tint

        ceilingTiles.push_back(ceiling);
    }
}


void GenerateSpiderWebs(float baseY)
{
    spiderWebs.clear();

    for (int y = 1; y < dungeonHeight - 1; y++) {
        for (int x = 1; x < dungeonWidth - 1; x++) {

            Color current = dungeonPixels[y * dungeonWidth + x];
            bool isWeb = (current.r == 128 && current.g == 128 && current.b == 128);  // light gray

            if (!isWeb) continue;

            // Check surrounding walls to determine orientation
            Color left = dungeonPixels[y * dungeonWidth + (x - 1)];
            Color right = dungeonPixels[y * dungeonWidth + (x + 1)];
            Color up = dungeonPixels[(y - 1) * dungeonWidth + x];
            Color down = dungeonPixels[(y + 1) * dungeonWidth + x];

            bool wallLeft  = (left.r == 0 && left.g == 0 && left.b == 0);
            bool wallRight = (right.r == 0 && right.g == 0 && right.b == 0);
            bool wallUp    = (up.r == 0 && up.g == 0 && up.b == 0);
            bool wallDown  = (down.r == 0 && down.g == 0 && down.b == 0);

            float rotationY = 0.0f;
            if (wallLeft && wallRight) {
                rotationY = 0;
            } 
            else if (wallUp && wallDown) {
                rotationY = 90.0f * DEG2RAD;
            } 
            else {
                continue;  // not valid web position
            }

            // World position
            Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);
            Vector3 offsetPos = {pos.x, pos.y + 150, pos.z};
            pos = offsetPos;

            // Define oriented bounding box (thin plane)
            float width = 150.0f;
            float thickness = 20.0f;
            float height = 200.0f;

            BoundingBox box;
            if (rotationY == 0.0f) {
                // Facing Z+ or Z-
                box.min = { pos.x - width * 0.5f, pos.y, pos.z - thickness * 0.5f };
                box.max = { pos.x + width * 0.5f, pos.y + height, pos.z + thickness * 0.5f };
            }
            else {
                // Facing X+ or X-
                box.min = { pos.x - thickness * 0.5f, pos.y, pos.z - width * 0.5f };
                box.max = { pos.x + thickness * 0.5f, pos.y + height, pos.z + width * 0.5f };
            }

            // Add to spiderWebs
            spiderWebs.push_back({
                pos,
                WHITE,
                box,
                false,
                rotationY
            });
        }
    }
}


// void GenerateSpiderWebs(float baseY) {
//     spiderWebs.clear();

//     for (int y = 0; y < dungeonHeight; y++) {
//         for (int x = 0; x < dungeonWidth; x++) {
//             Color current = dungeonPixels[y * dungeonWidth + x];

//             if (current.r == 128 && current.g == 128 && current.b == 128) { // light grey = spiderweb
//                 Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

//                 // Define bounding box as 200x200x200 cube centered on pos
//                 float halfSize = 100.0f;
//                 BoundingBox box;
//                 box.min = {
//                     pos.x - halfSize,
//                     pos.y,
//                     pos.z - halfSize
//                 };
//                 box.max = {
//                     pos.x + halfSize,
//                     pos.y + 100.0f,
//                     pos.z + halfSize
//                 };
                
//                 spiderWebs.push_back({pos, WHITE, box, false});

//             }
//         }
//     }
// }


void GenerateBarrels(float baseY) {
    barrelInstances.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 0 && current.g == 0 && current.b == 255) { // Blue = Barrel
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

                // Define bounding box as 100x100x100 cube centered on pos
                float halfSize = 50.0f;
                BoundingBox box;
                box.min = {
                    pos.x - halfSize,
                    pos.y,
                    pos.z - halfSize
                };
                box.max = {
                    pos.x + halfSize,
                    pos.y + 100.0f,
                    pos.z + halfSize
                };
                bool willContainPotion = (GetRandomValue(0, 99) < 25); // 25% chance
                bool willContainGold = (GetRandomValue(0, 99) < 50);
                if (willContainPotion) willContainGold = false;
                barrelInstances.push_back({
                    pos,
                    WHITE,
                    box,
                    false,
                    willContainPotion,
                    willContainGold
                });
                //barrelInstances.push_back({ pos, WHITE, box });
            }
        }
    }
}

void GenerateChests(float baseY) {
    chestInstances.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 0 && current.g == 128 && current.b == 255) { // SkyBlue = chest
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

                // Define bounding box as 100x100x100 cube centered on pos
                float halfSize = 10.0f;
                BoundingBox box;
                box.min = {
                    pos.x - halfSize,
                    pos.y,
                    pos.z - halfSize
                };
                box.max = {
                    pos.x + halfSize,
                    pos.y + 100.0f,
                    pos.z + halfSize
                };

                Model model = LoadModel("assets/models/chest.glb");

                int animCount = 0;
                ModelAnimation *anims = LoadModelAnimations("assets/models/chest.glb", &animCount);

                ChestInstance chest = {
                    model,
                    anims,
                    animCount,
                    pos,
                    WHITE,
                    box,
                    false, // open
                    false, // animPlaying
                    0.0f   // animFrame
                };

                chestInstances.push_back(chest);
                
            }
        }
    }
}


void GeneratePotions(float baseY) {
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 255 && current.g == 105 && current.b == 180) { // pink for potions
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY + 50); // raised slightly off floor
                Collectable p = {CollectableType::HealthPotion, pos, &healthPotTexture, 40};
                collectables.push_back(p);
            }
        }
    }
}

void GenerateKeys(float baseY) {
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 255 && current.g == 200 && current.b == 0) { // Gold for keys
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY + 50); // raised slightly off floor
                Collectable key = {CollectableType::Key, pos, &keyTexture, 100.0f};
                collectables.push_back(key);
            }
        }
    }
}


void GenerateRaptorsFromImage( float baseY) { //unused. no raptors allowed in dungeons, red means skeleton

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Look for pure red pixels (255, 0, 0)
            if (current.r == 255 && current.g == 0 && current.b == 0) {
                Vector3 spawnPos = GetDungeonWorldPos(x, y, tileSize, 135);
                
                Character raptor(spawnPos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);

                enemies.push_back(raptor);
                enemyPtrs.push_back(&enemies.back());
            }
        }
    }


}

void GenerateSpiderFromImage(float baseY) {
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Look for pure red pixels (255, 0, 0) → Skeleton spawn
            if (current.r == 64 && current.g == 64 && current.b == 64) {
                Vector3 spawnPos = GetDungeonWorldPos(x, y, tileSize, baseY);

                Character spider(
                    spawnPos,
                    &spiderSheet, 
                    200, 200,         // frame width, height
                    1,                // max frames
                    0.5f, 0.5f,       // scale, speed
                    0,                // initial animation frame
                    CharacterType::Spider
                );
                spider.maxHealth = 100;
                spider.currentHealth = 100; //at least 2 shots. 4 sword swings 
                
                enemies.push_back(spider);
                enemyPtrs.push_back(&enemies.back()); 
            }
        }
    }


}

void GenerateSkeletonsFromImage(float baseY) {


    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Look for pure red pixels (255, 0, 0) → Skeleton spawn
            if (current.r == 255 && current.g == 0 && current.b == 0) {
                Vector3 spawnPos = GetDungeonWorldPos(x, y, tileSize, baseY);

                Character skeleton(
                    spawnPos,
                    &skeletonSheet, 
                    200, 200,         // frame width, height
                    1,                // max frames
                    0.8f, 0.5f,       // scale, speed
                    0,                // initial animation frame
                    CharacterType::Skeleton
                );
                skeleton.maxHealth = 200;
                skeleton.currentHealth = 200; //at least 2 shots. 4 sword swings 
                
                enemies.push_back(skeleton);
                enemyPtrs.push_back(&enemies.back()); 
            }
        }
    }


}

void GeneratePiratesFromImage(float baseY) {


    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Look for magenta pixels (255, 0, 255) → Pirate spawn
            if (current.r == 255 && current.g == 0 && current.b == 255) {
                Vector3 spawnPos = GetDungeonWorldPos(x, y, tileSize, baseY);

                Character pirate(
                    spawnPos,
                    &pirateSheet, 
                    200, 200,         // frame width, height 
                    1,                // max frames, set when setting animations
                    0.5f, 0.5f,       // scale, speed
                    0,                // initial animation frame
                    CharacterType::Pirate
                );
                
                pirate.maxHealth = 400; // twice as tough as skeletons, at least 3 shots. 8 slices.
                pirate.currentHealth = 400;//bullets are 25 damage x 6 for the blunderbus. 150 if all the pellets hit 
                enemies.push_back(pirate);
                enemyPtrs.push_back(&enemies.back()); 
            }
        }
    }

}







void GenerateLightSources(float baseY) {
    dungeonLights.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Check for yellow (pure R + G, no B)
            if (current.r == 255 && current.g == 255 && current.b == 0) {
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

                dungeonLights.push_back({ pos, 1.0f });

                // Create a 100x100x100 bounding box centered on pos
                BoundingBox box;
                box.min = Vector3Subtract(pos, Vector3{50.0f, 0.0f, 50.0f});
                box.max = Vector3Add(pos, Vector3{50.0f, 100.0f, 50.0f});

                pillars.push_back({ pos, 1.0f, box });

                Fire newFire;
                newFire.fireFrame = GetRandomValue(0, 59);
                fires.push_back(newFire);
            }

        }
    }
}

Vector3 GetDungeonWorldPos(int x, int y, float tileSize, float baseY) {
    int flippedY = dungeonHeight - 1 - y;
    int flippedX = dungeonWidth - 1 - x;
    //flip x and y to match world coords. 
    return Vector3{
        flippedX * tileSize + tileSize / 2.0f, //center of the tile
        baseY,
        flippedY * tileSize + tileSize / 2.0f
    };
}

int GetDungeonImageX(float worldX, float tileSize, int dungeonWidth) {
    return dungeonWidth - 1 - (int)(worldX / tileSize);
}

int GetDungeonImageY(float worldZ, float tileSize, int dungeonHeight) {
    return dungeonHeight - 1 - (int)(worldZ / tileSize);
}

// for (const BarrelInstance& barrel : barrelInstances) {
//     Model modelToDraw = barrel.destroyed ? brokeBarrelModel : regularBarrelModel;
//     DrawModel(modelToDraw, barrel.position, 1.0f, barrel.tint);
// }



void DrawFlatWeb(Texture2D texture, Vector3 position, float width, float height, float rotationY, Color tint)
{
    // Compute 4 corners of the quad in local space
    Vector3 p1 = {-width/2, -height/2, 0};
    Vector3 p2 = { width/2, -height/2, 0};
    Vector3 p3 = { width/2,  height/2, 0};
    Vector3 p4 = {-width/2,  height/2, 0};

    // Apply Y-axis rotation
    Matrix rot = MatrixRotateY(rotationY);
    p1 = Vector3Transform(p1, rot);
    p2 = Vector3Transform(p2, rot);
    p3 = Vector3Transform(p3, rot);
    p4 = Vector3Transform(p4, rot);

    // Translate to world position
    p1 = Vector3Add(p1, position);
    p2 = Vector3Add(p2, position);
    p3 = Vector3Add(p3, position);
    p4 = Vector3Add(p4, position);

    // Draw the textured quad
    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        rlTexCoord2f(0, 0); rlVertex3f(p1.x, p1.y, p1.z);
        rlTexCoord2f(1, 0); rlVertex3f(p2.x, p2.y, p2.z);
        rlTexCoord2f(1, 1); rlVertex3f(p3.x, p3.y, p3.z);
        rlTexCoord2f(0, 1); rlVertex3f(p4.x, p4.y, p4.z);

    rlEnd();

    rlSetTexture(0);
}

void DrawSpiderWebs(Camera& camera) {
    for (const SpiderWebInstance& web : spiderWebs) {
        Texture2D webTexture = web.destroyed ? brokeWebTexture : spiderWebTexture;
        DrawFlatWeb(webTexture, web.position, 300.0f, 300.0f, web.rotationY, WHITE);
    }
}



void DrawDungeonBarrels() {
    for (const BarrelInstance& barrel : barrelInstances) {
        Vector3 offsetPos = {barrel.position.x, barrel.position.y + 20, barrel.position.z}; //move the barrel up a bit
        Model modelToDraw = barrel.destroyed ? brokeBarrel : barrelModel;
        DrawModelEx(modelToDraw, offsetPos, Vector3{0, 1, 0}, 0.0f, Vector3{350.0f, 350.0f, 350.0f}, barrel.tint); //scaled half size
        
    }
}

void DrawDungeonChests() {
    for (const ChestInstance& chest : chestInstances) {
        Vector3 offsetPos = {chest.position.x, chest.position.y + 20, chest.position.z};
        if (chest.animFrame > 0){
            offsetPos.z -= 45;
        }
        DrawModelEx(chest.model, offsetPos, Vector3{0, 1, 0}, 0.0f, Vector3{60.0f, 60.0f, 60.0f}, chest.tint);
    }
}




void DrawDungeonFloor() {
    for (const FloorTile& tile : floorTiles) {
        DrawModelEx(tile.floorTile, tile.position, Vector3{0,1,0}, 0.0f, Vector3{700,700,700}, tile.tint);
    }
}


void DrawDungeonWalls() {

    for (const WallInstance& _wall : wallInstances) {
        
        DrawModelEx(wall, _wall.position, Vector3{0, 1, 0}, _wall.rotationY, Vector3{700, 700, 700}, _wall.tint);

    }
}

void DrawDungeonDoorways(Model archwayModel){

    for (const DoorwayInstance& d : doorways) {
        Vector3 dPos = {d.position.x, d.position.y + 100, d.position.z};
        DrawModelEx(archwayModel, dPos, {0, 1, 0}, d.rotationY * RAD2DEG, {490, 595, 476}, d.tint);
    }

    for (const Door& door : doors){
   
        DrawFlatDoor(door);

    }
}

void DrawFlatDoor(const Door& door) {
    if (door.isOpen) return;

    float w = door.scale.x;
    float h = door.scale.y;

    // Determine local axes
    Vector3 forward = Vector3RotateByAxisAngle({0, 0, 1}, {0, 1, 0}, door.rotationY);
    Vector3 right = Vector3CrossProduct({0, 1, 0}, forward);

    // Use door.position directly as the center
    Vector3 center = door.position;

    // Compute quad corners (centered on door.position)
    Vector3 bottomLeft  = Vector3Add(center, Vector3Add(Vector3Scale(right, -w * 0.5f), Vector3Scale(forward, -door.scale.z * 0.5f)));
    Vector3 bottomRight = Vector3Add(center, Vector3Add(Vector3Scale(right,  w * 0.5f), Vector3Scale(forward, -door.scale.z * 0.5f)));
    Vector3 topLeft     = Vector3Add(bottomLeft, {0, h, 0});
    Vector3 topRight    = Vector3Add(bottomRight, {0, h, 0});

    rlSetTexture(door.doorTexture->id);
    rlBegin(RL_QUADS);
        rlColor4ub(door.tint.r, door.tint.g, door.tint.b, door.tint.a); //tint the door

        rlTexCoord2f(0, 1); rlVertex3f(bottomLeft.x,  bottomLeft.y,  bottomLeft.z);
        rlTexCoord2f(1, 1); rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
        rlTexCoord2f(1, 0); rlVertex3f(topRight.x,    topRight.y,    topRight.z);
        rlTexCoord2f(0, 0); rlVertex3f(topLeft.x,     topLeft.y,     topLeft.z);
    rlEnd();
    rlSetTexture(0);
    rlColor4ub(255, 255, 255, 255); // Reset color for next draw calls
}





void DrawDungeonCeiling(Model ceilingTileModel) {
    for (const CeilingTile& ceiling : ceilingTiles) {
        DrawModelEx(
            ceilingTileModel,
            ceiling.position,
            Vector3{1, 0, 0}, // Flip to face downward
            180.0f,
            Vector3{700, 700, 700},
            ceiling.tint
        );
    }
}



void DrawDungeonPillars(float deltaTime, Camera3D camera) {
    //Pillars means Pedestal fire light sources. Light sources are generated separatly and spawn at pillar positions. 
    for (size_t i = 0; i < pillars.size(); ++i) {
        const PillarInstance& pillar = pillars[i];
        Fire& fire = fires[i];

        // Draw the pedestal model
        DrawModelEx(lampModel, pillar.position, Vector3{0, 1, 0}, pillar.rotation, Vector3{350, 350, 350}, WHITE);

        // // Animate fire
        // fire.fireAnimTimer += deltaTime;
        // if (fire.fireAnimTimer >= fire.fireFrameDuration) {
        //     fire.fireAnimTimer -= fire.fireFrameDuration;
        //     fire.fireFrame = (fire.fireFrame + 1) % 60;
        // }

        // // Calculate frame position in sheet
        // int frameX = fire.fireFrame % 10;
        // int frameY = fire.fireFrame / 10;
        // Rectangle sourceRect = {
        //     (float)(frameX * 64),
        //     (float)(frameY * 64),
        //     64.0f,
        //     64.0f
        // };

        // // Position the fire above the bowl
        // Vector3 firePos = pillar.position;
        // firePos.y += 130; 

        // // Draw animated fire as billboard
        // Vector2 fireSize = {100, 100};
        // rlDisableDepthMask();
        // DrawBillboardRec(camera, fireSheet, sourceRect, firePos, fireSize, WHITE);
        // rlEnableDepthMask();
    }
}


void UpdateDoorwayTints(Vector3 playerPos) {
    const float playerLightRange = 1000.0f;
    const float minBrightness = 0.5f;

    for (DoorwayInstance& door : doorways) {
        // Player light
        float distToPlayer = Vector3Distance(playerPos, door.position);
        float brightness = Clamp(1.0f - (distToPlayer / playerLightRange), minBrightness, 1.0f);

        // Dynamic lights
        for (const LightSource& light : bulletLights){
            float distToLight = Vector3Distance(light.position, door.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);  
            brightness += lightContribution * light.intensity;
        }

        //static lights
        for (const LightSource& light : dungeonLights) {
            float distToLight = Vector3Distance(light.position, door.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);  // Uses light.range!
            brightness += lightContribution * light.intensity;
        }

        brightness = Clamp(brightness, minBrightness, 1.0f);

        Vector3 warmTint = Vector3{1.0f, 0.85f, 0.7f};
        Vector3 tinted = Vector3Scale(warmTint, brightness);
        door.tint = ColorFromNormalized({ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}

void UpdateWallTints(Vector3 playerPos) {
    const float playerLightRange = 1500.0f;
    const float minBrightness = 0.5f;

    for (WallInstance& wall : wallInstances) {
        // Player light
        float distToPlayer = Vector3Distance(playerPos, wall.position);
        float brightness = Clamp(1.0f - (distToPlayer / playerLightRange), minBrightness, 1.0f);

        // Dynamic lights
        for (const LightSource& light : bulletLights){
            float distToLight = Vector3Distance(light.position, wall.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);  
            brightness += lightContribution * light.intensity;
        }

        //static lights
        for (const LightSource& light : dungeonLights) {
            float distToLight = Vector3Distance(light.position, wall.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);  // Uses light.range!
            brightness += lightContribution * light.intensity;
        }

        brightness = Clamp(brightness, minBrightness, 1.0f);

        Vector3 warmTint = Vector3{1.0f, 0.85f, 0.7f};
        Vector3 tinted = Vector3Scale(warmTint, brightness);
        wall.tint = ColorFromNormalized({ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}



void UpdateFloorTints(Vector3 playerPos) {
    const float maxLightDistance = 1000.0f;
    const float minBrightness = 0.2f;

    for (FloorTile& tile : floorTiles) {
        // Player light
        float distToPlayer = Vector3Distance(playerPos, tile.position);
        float brightness = Clamp(1.0f - (distToPlayer / 1000), minBrightness, 1.0f);

            // Dynamic lights
        for (const LightSource& light : bulletLights){
            float distToLight = Vector3Distance(light.position, tile.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);  
            brightness += lightContribution * light.intensity;
        }


        // Other lights
        for (const LightSource& light : dungeonLights) {
            float distToLight = Vector3Distance(light.position, tile.position);
            float lightContribution = Clamp(1.0f - (distToLight / maxLightDistance), 0.0f, 1.0f);
            brightness += lightContribution * light.intensity;
        }

        brightness = Clamp(brightness, minBrightness, 1.0f);  // Clamp final brightness

        // Warm tint (same as walls)
        Vector3 warmTint = Vector3{1.0f, 0.85f, 0.7f};
        Vector3 tinted = Vector3Scale(warmTint, brightness);
        tile.tint = ColorFromNormalized({ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}




void UpdateCeilingTints(Vector3 playerPos) {
    const float playerLightRange = 1500.0f;
    const float minBrightness = 0.2f;

    Vector3 warmCeilingColor = {0.7f, 0.6f, 0.5f}; // slightly muted warm tone

    for (CeilingTile& ceiling : ceilingTiles) {
        // Player light
        float distToPlayer = Vector3Distance(playerPos, ceiling.position);
        float brightness = Clamp(1.0f - (distToPlayer / playerLightRange), minBrightness, 1.0f);

                // Dynamic lights
        for (const LightSource& light : bulletLights){
            float distToLight = Vector3Distance(light.position, ceiling.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);  
            brightness += lightContribution * light.intensity;
        }


        // Other light sources
        for (const LightSource& light : dungeonLights) {
            float distToLight = Vector3Distance(light.position, ceiling.position);
            float lightContribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);
            brightness += lightContribution * light.intensity;
        }

        brightness = Clamp(brightness, minBrightness, 1.0f);
        Vector3 tinted = Vector3Scale(warmCeilingColor, brightness);
        ceiling.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}



void UpdateChestTints(Vector3 playerPos) {
    const float maxLightDistance = 4000.0f;
    const float minBrightness = 0.2f;

    Vector3 warmchestColor = {0.9f, 0.7f, 0.6f}; // slightly muted warm tone

    for (ChestInstance& chest : chestInstances) {
        float dist = Vector3Distance(playerPos, chest.position);
        float brightness = Clamp(1.0f - (dist / maxLightDistance), minBrightness, 1.0f);
        Vector3 tinted = Vector3Scale(warmchestColor, brightness);
        chest.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}


void UpdateBarrelTints(Vector3 playerPos) {
    const float maxLightDistance = 4000.0f;
    const float minBrightness = 0.2f;

    Vector3 warmbarrelColor = {0.9f, 0.7f, 0.6f}; // slightly muted warm tone

    for (BarrelInstance& barrel : barrelInstances) {
        float dist = Vector3Distance(playerPos, barrel.position);
        float brightness = Clamp(1.0f - (dist / maxLightDistance), minBrightness, 1.0f);
        Vector3 tinted = Vector3Scale(warmbarrelColor, brightness);
        barrel.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}

void UpdateDoorTints(Vector3 playerPos) {
    const float maxLightDistance = 4000.0f;
    const float minBrightness = 0.2f;

    Vector3 baseDoorColor = {0.8f, 0.6f, 0.4f}; // warm wood tone

    for (Door& door : doors) {
        float dist = Vector3Distance(playerPos, door.position);
        float brightness = Clamp(1.0f - (dist / maxLightDistance), minBrightness, 1.0f);
        Vector3 tinted = Vector3Scale(baseDoorColor, brightness);
        door.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}


bool IsDungeonFloorTile(int x, int y) {
    if (x < 0 || x >= dungeonWidth || y < 0 || y >= dungeonHeight) return false;
    Color c = dungeonPixels[y * dungeonWidth + x];
    return c.r > 200 && c.g > 200 && c.b > 200;  // close to white
}



BoundingBox MakeDoorBoundingBox(Vector3 position, float rotationY, float halfWidth, float height, float depth) {
    //covers the full archway
    Vector3 forward = Vector3RotateByAxisAngle({0, 0, 1}, {0, 1, 0}, rotationY);
    Vector3 right = Vector3CrossProduct({0, 1, 0}, forward);

    // Calculate combined half extents
    Vector3 halfExtents = Vector3Add(
        Vector3Scale(right, halfWidth),
        Vector3Scale(forward, depth)
    );

    Vector3 boxMin = {
        position.x - fabsf(halfExtents.x),
        position.y,
        position.z - fabsf(halfExtents.z)
    };

    Vector3 boxMax = {
        position.x + fabsf(halfExtents.x),
        position.y + height,
        position.z + fabsf(halfExtents.z)
    };

    return { boxMin, boxMax };
}



void ClearDungeon() {
    wallRunColliders.clear();
    floorTiles.clear();
    wallInstances.clear();
    ceilingTiles.clear();
    pillars.clear();
    barrelInstances.clear();
    dungeonLights.clear();
    enemies.clear();
    enemyPtrs.clear();
    doors.clear();
    doorways.clear();
    collectables.clear();
    for (ChestInstance& chest : chestInstances) {
        UnloadModel(chest.model);
        UnloadModelAnimations(chest.animations, chest.animCount);
    }
    chestInstances.clear();

}

