#include "dungeonGeneration.h"
#include "raymath.h"
#include <iostream>
#include "player.h"
#include "vector"
#include "world.h"
#include "rlgl.h"
#include "sound_manager.h"
#include "transparentDraw.h"
#include "pathfinding.h"
#include "resourceManager.h"
#include "utilities.h"

std::vector<LauncherTrap> launchers;
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
std::vector<LightSource> bulletLights; //fireball/iceball
std::vector<Fire> fires;

Image dungeonImg; //save the dungeon info globaly
Color* dungeonPixels = nullptr;
int dungeonWidth = 0;
int dungeonHeight = 0;


float playerLightRange = 800.0f;
float playerLightIntensity = 0.5f;

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

//    light gray = spiderWeb (128, 128, 128)

//    Dark Red = magicStaff (128, 0, 0)

//    very light grey = ghost (200, 200, 200)

//    Vermillion = launcherTrap (255, 66, 52)

//    yellowish = direction pixel (200, 200, 0)

//    medium yellow = timing pixel (200, 50, 0)/ (200, 100, 0)/ (200, 150, 0)



Vector3 ColorToNormalized(Color color) {
    return (Vector3){
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f
    };
}

float ColorAverage(Color c) {
    return ((c.r + c.g + c.b) / 3.0f) / 255.0f;
}

void GenerateWeapons(float Height){
    worldWeapons.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 128 && current.g == 0 && current.b == 0) { // Dark red staff
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, Height);
                worldWeapons.push_back(CollectableWeapon(WeaponType::MagicStaff, pos, R.GetModel("staffModel")));

            }
        }
    }
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
            chest.animFrame += GetFrameTime() * 50.0f;

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
            Collectable key(CollectableType::Key, pos, R.GetTexture("keyTexture"), 100);
            
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
            //tile.floorTile = &floorTile;


            floorTiles.push_back(tile);
        }
    }
}





// === Helper Functions ===
inline bool ColorsEqual(Color a, Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

inline bool IsWallColor(Color c) {
    return ColorsEqual(c, { 0, 0, 0, 255 });
}

inline bool IsBarrelColor(Color c) {
    return ColorsEqual(c, { 0, 0, 255, 255 });
}

inline bool IsEnemyColor(Color c) {
    return ColorsEqual(c, { 255, 0, 0, 255 });
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
        Door door{};
        door.position = dw.position;
        door.rotationY = dw.rotationY + DEG2RAD * 90.0f;
        door.isOpen = false;struct DoorwayInstance {
    Vector3 position;
    float rotationY;
    bool isOpen = false;
    bool isLocked = false;
    Color tint = GRAY;
    Color bakedTint;
    float bakedBrightness;
    int tileX;
    int tileY;
    int linkedLevelIndex = -1;
    std::vector<BoundingBox> sideColliders;

};
        door.isLocked = dw.isLocked;
        door.doorTexture = R.GetTexture("doorTexture");
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
            Vector3 offsetPos = {pos.x, pos.y + 200, pos.z};
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

inline bool IsDirPixel(Color c) {
    return c.r == 200 && c.g == 200 && c.b == 0;      // direction (yellow-ish)
}

inline bool IsTimingPixel(Color c) {
    if (c.r != 200 || c.b != 0) return false;
    return (c.g == 50 || c.g == 100 || c.g == 150);
}

inline float TimingFromPixel(Color c) {
    // 50 -> 1s, 100 -> 2s, 150 -> 3s
    return c.g / 50.0f;
}

void GenerateLaunchers(float baseY) {
    launchers.clear();

    const int dx[4] = { -1, 1, 0, 0 };
    const int dy[4] = {  0, 0,-1, 1 };

    for (int y = 0; y < dungeonHeight; ++y) {
        for (int x = 0; x < dungeonWidth; ++x) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Vermilion trap pixel
            if (!(current.r == 255 && current.g == 66 && current.b == 52)) continue;

            float rotDeg = 0.0f;
            float fireIntervalSec = 1.0f; // sensible default
            bool found = false;

            // Find the yellow direction pixel among the 4 neighbors
            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i], ny = y + dy[i];
                if (nx < 0 || nx >= dungeonWidth || ny < 0 || ny >= dungeonHeight) continue;

                Color neighbor = dungeonPixels[ny * dungeonWidth + nx];
                if (!IsDirPixel(neighbor)) continue;

                // Map neighbor offset -> yaw
                if      (dx[i] == 0  && dy[i] == -1) rotDeg =   0.0f;  // up    -> +Z
                else if (dx[i] == 1  && dy[i] ==  0) rotDeg =  90.0f;  // right -> +X
                else if (dx[i] == 0  && dy[i] ==  1) rotDeg = 180.0f;  // down  -> -Z
                else if (dx[i] == -1 && dy[i] ==  0) rotDeg = 270.0f;  // left  -> -X

                // Timing pixel should be on the *opposite* side: (x - dx[i], y - dy[i])
                int tx = x - dx[i], ty = y - dy[i];
                if (tx >= 0 && tx < dungeonWidth && ty >= 0 && ty < dungeonHeight) {
                    Color timing = dungeonPixels[ty * dungeonWidth + tx];
                    if (IsTimingPixel(timing)) {
                        fireIntervalSec = TimingFromPixel(timing);
                    }
                }

                found = true;
                break;
            }

            // Build trap
            Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

            float halfSize = 50.0f;
            BoundingBox box;
            box.min = { pos.x - halfSize, pos.y,          pos.z - halfSize };
            box.max = { pos.x + halfSize, pos.y + 100.0f, pos.z + halfSize };

            std::cout << "Launcher at (" << x << ", " << y << ") "
                    << "rotDeg=" << rotDeg << " "
                    << "interval=" << fireIntervalSec << "s"
                    << std::endl;

            launchers.push_back({ TrapType::fireball, pos, rotDeg, fireIntervalSec, box });
        }
    }
}



void GenerateBarrels(float baseY) {
    barrelInstances.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 0 && current.g == 0 && current.b == 255) { // Blue = Barrel
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

                // Define bounding box as 100x100x100 cube centered on pos, tileSize is 200 so half tile size centered. 
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
                //Decide what the barrel will drop. 
                int roll = GetRandomValue(0, 99);
                bool willContainPotion = false;
                bool willContainMana = false;
                bool willContainGold = false;
                //barrels only drop one thing at a time. 
                if (roll < 25) {
                    willContainPotion = true;     // 0 - 24 → 25%
                } else if (roll < 35) {
                    willContainMana = true;       // 25 - 34 → 10%
                } else if (roll < 85) {
                    willContainGold = true;       // 35 - 84 → 50%
                }
                // 85 - 99 → 15% chance barrel contains nothing
                
                barrelInstances.push_back({
                    pos,
                    WHITE,
                    box,
                    false,
                    willContainPotion,
                    willContainGold,
                    willContainMana,
                    
                });
                
            }
        }
    }
}

void GenerateChests(float baseY) {
    chestInstances.clear();
    int chestID = 0;
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

                                // create a unique key for this chest model
                std::string key = "chestModel#" + std::to_string(chestID++);

                // load a _separate_ model for this chest
                // (this reads the same GLB but gives you independent skeleton data)
                R.LoadModel(key, "assets/models/chest.glb");
                Model& model = R.GetModel(key);

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
                Collectable p = {CollectableType::HealthPotion, pos, R.GetTexture("healthPotTexture"), 40};
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
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY + 80); // raised slightly off floor
                Collectable key = {CollectableType::Key, pos, R.GetTexture("keyTexture"), 100.0f};
                collectables.push_back(key);
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
                    R.GetTexture("spiderSheet"), 
                    200, 200,         // frame width, height
                    1,                // max frames
                    0.5f, 0.5f,       // scale, speed
                    0,                // initial animation frame
                    CharacterType::Spider
                );
                spider.maxHealth = 100;
                spider.currentHealth = 100; //2 sword attacks
                
                enemies.push_back(spider);
                enemyPtrs.push_back(&enemies.back()); 
            }
        }
    }

}

void GenerateGhostsFromImage(float baseY) {
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            
            if (current.r == 200 && current.g == 200 && current.b == 200) { //very light gray = ghost. 
                Vector3 spawnPos = GetDungeonWorldPos(x, y, tileSize, baseY);

                Character ghost(
                    spawnPos,
                    R.GetTexture("ghostSheet"), 
                    200, 200,         // frame width, height
                    1,                // max frames
                    0.8f, 0.5f,       // scale, speed
                    0,                // initial animation frame
                    CharacterType::Ghost
                );
                ghost.maxHealth = 200;
                ghost.currentHealth = 200; 

                enemies.push_back(ghost);
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
                    R.GetTexture("skeletonSheet"), 
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
                    R.GetTexture("pirateSheet"), 
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

                dungeonLights.push_back({ pos });

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
    //returns world position, centered on tile. 
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



void DrawLaunchers() {
    for (const LauncherTrap& launcher : launchers) {

        Vector3 offsetPos = {launcher.position.x, launcher.position.y + 20, launcher.position.z}; 
        DrawModelEx(R.GetModel("stonePillar"), offsetPos, Vector3{0,1,0}, launcher.rotation, Vector3{100, 100, 100}, WHITE);
    }

}



void DrawDungeonBarrels() {
    for (const BarrelInstance& barrel : barrelInstances) {
        Vector3 offsetPos = {barrel.position.x, barrel.position.y + 20, barrel.position.z}; //move the barrel up a bit
        Model modelToDraw = barrel.destroyed ? R.GetModel("brokeBarrel") : R.GetModel("barrelModel");
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
        DrawModelEx(R.GetModel("floorTile"), tile.position, Vector3{0,1,0}, 0.0f, Vector3{700,700,700}, tile.tint);
    }
}


void DrawDungeonWalls() {

    for (const WallInstance& _wall : wallInstances) {
        
        DrawModelEx(R.GetModel("wall"), _wall.position, Vector3{0, 1, 0}, _wall.rotationY, Vector3{700, 700, 700}, _wall.tint);

    }
}

void DrawDungeonDoorways(){

    for (const DoorwayInstance& d : doorways) {
        Vector3 dPos = {d.position.x, d.position.y + 100, d.position.z};
        DrawModelEx(R.GetModel("doorWay"), dPos, {0, 1, 0}, d.rotationY * RAD2DEG, {490, 595, 476}, d.tint);
    }

    // for (const Door& door : doors){
   
    //     DrawFlatDoor(door);

    // }
}

void DrawFlatDoor(Texture2D tex, Vector3 pos, float width, float height, float rotY, Color tint) {
    float w = width;
    float h = height;

    // Determine local axes
    Vector3 forward = Vector3RotateByAxisAngle({0, 0, 1}, {0, 1, 0}, rotY);
    Vector3 right = Vector3CrossProduct({0, 1, 0}, forward);

    // Use pos directly as the center
    Vector3 center = pos;

    // Compute quad corners (centered on position)
    Vector3 bottomLeft  = Vector3Add(center, Vector3Add(Vector3Scale(right, -w * 0.5f), Vector3Scale(forward, -1.0f)));
    Vector3 bottomRight = Vector3Add(center, Vector3Add(Vector3Scale(right,  w * 0.5f), Vector3Scale(forward, -1.0f)));
    Vector3 topLeft     = Vector3Add(bottomLeft, {0, h, 0});
    Vector3 topRight    = Vector3Add(bottomRight, {0, h, 0});

    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        rlTexCoord2f(0, 1); rlVertex3f(bottomLeft.x,  bottomLeft.y,  bottomLeft.z);
        rlTexCoord2f(1, 1); rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
        rlTexCoord2f(1, 0); rlVertex3f(topRight.x,    topRight.y,    topRight.z);
        rlTexCoord2f(0, 0); rlVertex3f(topLeft.x,     topLeft.y,     topLeft.z);
    rlEnd();
    rlSetTexture(0);
    rlColor4ub(255, 255, 255, 255);
}


void DrawDungeonCeiling() {
    for (const CeilingTile& ceiling : ceilingTiles) {
        DrawModelEx(
            R.GetModel("floorTile"),
            ceiling.position,
            Vector3{1, 0, 0}, // Flip to face downward
            180.0f,
            Vector3{700, 700, 700},
            ceiling.tint
        );
    }
}



void DrawDungeonPillars() {
    //Pillars means Pedestal fire light sources. Light sources are generated separatly and spawn at pillar positions. 
    for (size_t i = 0; i < pillars.size(); ++i) {
        const PillarInstance& pillar = pillars[i];
        //Fire& fire = fires[i];

        // Draw the pedestal model
        DrawModelEx(R.GetModel("lampModel"), pillar.position, Vector3{0, 1, 0}, pillar.rotation, Vector3{350, 350, 350}, WHITE);

    }
}

void HandleDungeonTints() {
     //Model Color Lighting
    // === Update tints ===

    UpdateWallTints(player.position);
    UpdateCeilingTints(player.position);
    UpdateFloorTints(player.position);
    UpdateBarrelTints(player.position);
    UpdateChestTints(player.position);
    UpdateDoorwayTints(player.position);
    UpdateDoorTints(player.position);
}

void ResetAllBakedTints() {
    for (auto& wall : wallInstances)
        wall.bakedTint = ColorFromNormalized({0.0f, 0.0f, 0.0f, 1.0f});

    for (auto& floor : floorTiles)
        floor.bakedTint = ColorFromNormalized({0.0f, 0.0f, 0.0f, 1.0f});

    for (auto& ceiling : ceilingTiles)
        ceiling.bakedTint = ColorFromNormalized({0.0f, 0.0f, 0.0f, 1.0f});

    for (auto& door : doorways)
        door.bakedTint = ColorFromNormalized({0.0f, 0.0f, 0.0f, 1.0f});
}


void BakeStaticLighting() {
    Vector3 warmTint = {0.7f, 0.7f, 0.7f}; //Light Gray 
    float brightnessScale = 1.25f; // Try values between 0.1 and 0.5
    
    float ambientBrightness = 0.3f;
    const float ambientFloorBrightness = 0.3;
    float epsilon = 0.25f;
    if (!isDungeon) ambientBrightness = 1.0f;

    for (WallInstance& wall : wallInstances) {
        wall.bakedBrightness = ambientBrightness;
    }

    for (DoorwayInstance& door : doorways){
        door.bakedBrightness = ambientBrightness;
    }

    for (FloorTile& floor : floorTiles){
        floor.bakedBrightness = ambientFloorBrightness;
    }

    for (CeilingTile& ceiling : ceilingTiles){
        ceiling.bakedBrightness = ambientFloorBrightness;
    }

    for (const LightSource& light : dungeonLights) {
        for (WallInstance& wall : wallInstances) {
            float dist = Vector3Distance(light.position, wall.position);
            if (dist > light.range) continue;
            if (!HasWorldLineOfSight(light.position, wall.position, epsilon)) continue;
            float t = Clamp(dist / light.range, 0.0f, 1.0f);
            float contribution = (1.0f - (t * t * (3 - 2 * t))) * light.intensity;
            
            wall.bakedBrightness += contribution * light.intensity;
        }

        for (FloorTile& floor : floorTiles){
            
            float dist = Vector3Distance(light.position, floor.position);
            if (dist > light.range) continue;
            if (!HasWorldLineOfSight(light.position, floor.position, epsilon)) continue;
            float t = Clamp(dist / (light.range - 700), 0.0f, 1.0f); //half the range for floors looks better. 
            float contribution = (1.0f - (t * t * (3 - 2 * t))) * light.intensity;
            //float contribution = Clamp(1.0f - (dist / light.range), 0.0f, 1.0f);
            floor.bakedBrightness += contribution * light.intensity;
        }

        for (DoorwayInstance& door : doorways){
            float dist = Vector3Distance(light.position, door.position);
            if (dist > light.range) continue;
            if (!HasWorldLineOfSight(light.position, door.position, epsilon)) continue;

            float contribution = Clamp(1.0f - (dist / light.range), 0.0f, 1.0f);
            door.bakedBrightness += contribution * light.intensity;
        }
    }

    //GAMMA CORRECTION
    for (WallInstance& wall : wallInstances) {
        float scaledBrightness = Clamp(wall.bakedBrightness * brightnessScale, 0.0f, 1.0f);
        Vector3 tinted = Vector3Scale(warmTint, scaledBrightness);

        float gamma = 1.25f;
        Vector3 gammaCorrected = {
            powf(tinted.x, 1.0f / gamma),
            powf(tinted.y, 1.0f / gamma),
            powf(tinted.z, 1.0f / gamma)
        };

        wall.bakedTint = ColorFromNormalized({ gammaCorrected.x, gammaCorrected.y, gammaCorrected.z, 1.0f });
    }

    for (DoorwayInstance& door : doorways) {
        float scaledBrightness = Clamp(door.bakedBrightness * brightnessScale, 0.0f, 1.0f);
        Vector3 tinted = Vector3Scale(warmTint, scaledBrightness);

        float gamma = 1.25f;
        Vector3 gammaCorrected = {
            powf(tinted.x, 1.0f / gamma),
            powf(tinted.y, 1.0f / gamma),
            powf(tinted.z, 1.0f / gamma)
        };

        door.bakedTint = ColorFromNormalized({ gammaCorrected.x, gammaCorrected.y, gammaCorrected.z, 1.0f });
    }

    
    for (FloorTile& floor : floorTiles) {
        float scaledBrightness = Clamp(floor.bakedBrightness * brightnessScale, 0.0f, 1.0f);
        Vector3 tinted = Vector3Scale({ .9f, 0.0f, 0.9f }, scaledBrightness);

        float gamma = 0.8f;
        Vector3 gammaCorrected = {
            powf(tinted.x, 1.0f / gamma),
            powf(tinted.y, 1.0f / gamma),
            powf(tinted.z, 1.0f / gamma)
        };

        floor.bakedTint = ColorFromNormalized({ gammaCorrected.x, gammaCorrected.y, gammaCorrected.z, 1.0f });
    }

    //copy floor baked light to ceiling. No need to recalculate cause it's the same. 
    if (floorTiles.size() != ceilingTiles.size()) {
        TraceLog(LOG_WARNING, "BakeStaticLighting: floorTiles and ceilingTiles size mismatch! Skipping ceiling bake.");
    } else {
        for (size_t i = 0; i < floorTiles.size(); ++i) {
            ceilingTiles[i].bakedTint = floorTiles[i].bakedTint;
        }
    }

}


void ApplyBakedLighting() {
    for (WallInstance& wall : wallInstances) {
        wall.tint = wall.bakedTint;
    }

    for (FloorTile& floor : floorTiles){
        floor.tint = floor.bakedTint;

    }

    for (CeilingTile& ceiling : ceilingTiles){
        ceiling.tint = ceiling.bakedTint;
    }

    for (DoorwayInstance& door : doorways){
        door.tint = door.bakedTint;
    }
}




void UpdateDoorwayTints(Vector3 playerPos) {
    if (!isDungeon) return; //dont tint outside doorways
    for (DoorwayInstance& wall : doorways) {
        // 1️⃣ Start with baked brightness
        float brightness = ColorAverage(wall.bakedTint);

        // Start with baked tint as base color
        Vector3 finalColor = Vector3Scale({1.0f, 0.85f, 0.7f}, brightness);  // base warm tint

        // 2️⃣ Player light contribution
        float distToPlayer = Vector3Distance(playerPos, wall.position);
        float playerContribution = Clamp(1.0f - (distToPlayer / playerLightRange), 0.0f, 1.0f);
        float playerLight = playerContribution * playerLightIntensity;
        finalColor = Vector3Add(finalColor, Vector3Scale({0.7f, 0.7f, 0.7f}, playerLight)); 

        // 3️⃣ Bullet lights (fireball or iceball, etc.)
        for (const LightSource& light : bulletLights) {
            float distToLight = Vector3Distance(light.position, wall.position);
            if (distToLight > light.range) continue;

            float contribution = Clamp(1.0f - (distToLight / light.range), 0.0f, 1.0f);
            float brightnessFromLight = contribution * light.fireballIntensity;

            finalColor = Vector3Add(finalColor, Vector3Scale(light.colorTint, brightnessFromLight));
        }

        // 4️⃣ Clamp final color to avoid overbright
        finalColor.x = Clamp(finalColor.x, 0.0f, 1.0f);
        finalColor.y = Clamp(finalColor.y, 0.0f, 1.0f);
        finalColor.z = Clamp(finalColor.z, 0.0f, 1.0f);

        // 5️⃣ Store to wall.tint
        wall.tint = ColorFromNormalized((Vector4){ finalColor.x, finalColor.y, finalColor.z, 1.0f });
    }
}


void UpdateWallTints(Vector3 playerPos) {
    for (WallInstance& wall : wallInstances) {
        // 1️⃣ Start with baked brightness
        float brightness = ColorAverage(wall.bakedTint);

        // Start with baked tint as base color
        Vector3 finalColor = Vector3Scale({1.0f, 0.85f, 0.7f}, brightness);  // base warm tint

        // 2️⃣ Player light contribution
        float distToPlayer = Vector3Distance(playerPos, wall.position);
        float playerContribution = Clamp(1.0f - (distToPlayer / playerLightRange), 0.0f, 1.0f);
        float playerLight = playerContribution * playerLightIntensity;
        finalColor = Vector3Add(finalColor, Vector3Scale({0.7f, 0.7f, 0.7f}, playerLight));  // same warm tint

        // 3️⃣ Bullet lights (fireball or iceball, etc.)
        for (const LightSource& light : bulletLights) {
            float distToLight = Vector3Distance(light.position, wall.position);
            if (distToLight > light.range) continue;

            float contribution = Clamp(1.0f - (distToLight / light.fireballRange), 0.0f, 1.0f);
            float brightnessFromLight = contribution * light.fireballIntensity;

            finalColor = Vector3Add(finalColor, Vector3Scale(light.colorTint, brightnessFromLight));
        }

        // 4️⃣ Clamp final color to avoid overbright
        finalColor.x = Clamp(finalColor.x, 0.0f, 1.0f);
        finalColor.y = Clamp(finalColor.y, 0.0f, 1.0f);
        finalColor.z = Clamp(finalColor.z, 0.0f, 1.0f);

        // 5️⃣ Store to wall.tint
        wall.tint = ColorFromNormalized((Vector4){ finalColor.x, finalColor.y, finalColor.z, 1.0f });
    }
}



void UpdateFloorTints(Vector3 playerPos) {
    for (FloorTile& tile : floorTiles) {
        float brightness = ColorAverage(tile.bakedTint);

        // Accumulate final color contribution per tile
        Vector3 finalColor = Vector3Scale({1.0f, .85f, 0.7f}, brightness); // baked tint as base

        // Player light
        float distToPlayer = Vector3Distance(playerPos, tile.position);
        float playerContribution = Clamp(1.0f - (distToPlayer / playerLightRange), 0.0f, 1.0f);
        float playerLight = playerContribution * playerLightIntensity;
        finalColor = Vector3Add(finalColor, Vector3Scale({1.0f, 0.85f, 0.7f}, playerLight)); // assume player light is warm

        // Dynamic lights
        for (const LightSource& light : bulletLights) {
            float distToLight = Vector3Distance(light.position, tile.position);
            if (distToLight > light.range) continue;

            float lightContribution = Clamp(1.0f - (distToLight / light.fireballRange), 0.0f, 1.0f);
            float lightBrightness = lightContribution * light.fireballIntensity;

            // Add this light’s color scaled by brightness
            finalColor = Vector3Add(finalColor, Vector3Scale(light.colorTint, lightBrightness));
        }

        // Clamp color channels
        finalColor.x = Clamp(finalColor.x, 0.0f, 1.0f);
        finalColor.y = Clamp(finalColor.y, 0.0f, 1.0f);
        finalColor.z = Clamp(finalColor.z, 0.0f, 1.0f);

        tile.tint = ColorFromNormalized({ finalColor.x, finalColor.y, finalColor.z, 1.0f });
    }
}





void UpdateCeilingTints(Vector3 playerPos) {
    for (CeilingTile& tile : ceilingTiles) {
        float brightness = ColorAverage(tile.bakedTint);

        // Accumulate final color contribution per tile
        Vector3 finalColor = Vector3Scale({1.0f, 0.85f, 0.7f}, brightness); // baked tint as base

        // Player light
        float distToPlayer = Vector3Distance(playerPos, tile.position);
        float playerContribution = Clamp(1.0f - (distToPlayer / playerLightRange), 0.0f, 1.0f);
        float playerLight = playerContribution * playerLightIntensity;
        finalColor = Vector3Add(finalColor, Vector3Scale({1.0f, 0.85f, 0.7f}, playerLight)); // assume player light is warm

        // Dynamic lights
        for (const LightSource& light : bulletLights) {
            float distToLight = Vector3Distance(light.position, tile.position);
            if (distToLight > light.range) continue;

            float lightContribution = Clamp(1.0f - (distToLight / light.fireballRange), 0.0f, 1.0f);
            float lightBrightness = lightContribution * light.fireballIntensity;

            // Add this light’s color scaled by brightness
            finalColor = Vector3Add(finalColor, Vector3Scale(light.colorTint, lightBrightness));
        }

        // Clamp color channels
        finalColor.x = Clamp(finalColor.x, 0.0f, 1.0f);
        finalColor.y = Clamp(finalColor.y, 0.0f, 1.0f);
        finalColor.z = Clamp(finalColor.z, 0.0f, 1.0f);

        tile.tint = ColorFromNormalized({ finalColor.x, finalColor.y, finalColor.z, 1.0f });
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
    if (!isDungeon) return;
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
    doors.clear();
    doorways.clear();
    collectables.clear();
    decals.clear();
   
    for (ChestInstance& chest : chestInstances) {
        UnloadModelAnimations(chest.animations, chest.animCount);
    }
    chestInstances.clear();
   
}

