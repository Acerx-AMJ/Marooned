#include "dungeonGeneration.h"
#include "raymath.h"
#include "resources.h"
#include <iostream>
#include "player.h"
#include "vector"
#include "world.h"
#include "rlgl.h"
//static std::vector<Vector3> floorTilePositions;
std::vector<FloorTile> floorTiles;

static std::vector<WallInstance> wallInstances;

std::vector<CeilingTile> ceilingTiles;
std::vector<BarrelInstance> barrelInstances;
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

//⬛ Black = Wall

//⬜ White = Floor (the whole dungeon is filled with floor not just white pixels)

//🟩 Green = Player Start

//🔵 Blue = Barrel

//🟥 Red = Skeleton

//🟨 Yellow = Light

//🟪 Purple = Doorway (128, 0, 128)

//  Teal = Dungeon Exit (0, 128, 128)

//🟧 Orange = Next Level (255, 128, 0)

//  Aqua = locked door (0, 255, 255)

//💗  Pink = health pot (255, 105, 180)

//⭐  Gold = key (255, 200, 0)


    

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

    max.y += height + 1000; // dont jump over walls. 1000 high

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



void GenerateFloorTiles(float tileSize, float baseY) {
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




void GenerateWallTiles(float tileSize, float baseY) {
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

void GenerateDoorways(float tileSize, float baseY, int currentLevelIndex) {
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
        
        float halfWidth = 200.0f;   // Half of the 400-unit wide doorway
        float height = 365.0f;
        float depth = 20.0f;        // Thickness into the doorway (forward axis)

        door.collider = MakeDoorBoundingBox(door.position, door.rotationY, halfWidth, height, depth);

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


void GenerateBarrels(float tileSize, float baseY) {
    barrelInstances.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 0 && current.g == 0 && current.b == 255) {
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);
                barrelInstances.push_back({ pos });
            }
        }
    }
}

void GeneratePotions(float tileSize, float baseY) {
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 255 && current.g == 105 && current.b == 180) { // pink for potions
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY + 50); // raised slightly off floor
                collectables.push_back(Collectable(CollectableType::HealthPotion, pos));
            }
        }
    }
}

void GenerateKeys(float tileSize, float baseY) {
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            if (current.r == 255 && current.g == 200 && current.b == 0) { // Gold for keys
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY + 50); // raised slightly off floor
                collectables.push_back(Collectable(CollectableType::Key, pos));
            }
        }
    }
}


void GenerateRaptorsFromImage(float tileSize, float baseY) { //unused. no raptors allowed in dungeons, red means skeleton
    raptors.clear(); // Clear existing raptors
    raptorPtrs.clear();
    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Look for pure red pixels (255, 0, 0)
            if (current.r == 255 && current.g == 0 && current.b == 0) {
                Vector3 spawnPos = GetDungeonWorldPos(x, y, tileSize, 135);
                
                Character raptor(spawnPos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);

                raptors.push_back(raptor);
            }
        }
    }

    for (Character& r : raptors) { //don't forget raptorPtrs
        raptorPtrs.push_back(&r);
    }
}

void GenerateSkeletonsFromImage(float tileSize, float baseY) {
    skeletons.clear();
    skeletonPtrs.clear();

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
                    0.5f, 0.5f,       // scale, speed
                    0,                // initial animation frame
                    CharacterType::Skeleton
                );
                skeleton.maxHealth = 200;
                skeleton.currentHealth = 200; //at least 2 shots. 4 sword swings 
                skeletons.push_back(skeleton);
            }
        }
    }

    for (Character& s : skeletons) { //Ptrs for sorting
        skeletonPtrs.push_back(&s);
    }
}



void GenerateLightSources(float tileSize, float baseY) {
    dungeonLights.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];

            // Check for yellow (pure R + G, no B)
            if (current.r == 255 && current.g == 255 && current.b == 0) {
             
                Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);
                Vector3 pPos = GetDungeonWorldPos(x, y, tileSize, baseY);

                dungeonLights.push_back({ pos, 1.0f }); // 1.0f = full intensity
                pillars.push_back({pPos, 1.0f}); //put a pillar wherever there is a lightsource, for now

                Fire newFire;
                newFire.fireFrame = GetRandomValue(0, 59); // Optional: desync the flames
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




void DrawDungeonBarrels(Model barrelModel) {
    for (const BarrelInstance& barrel : barrelInstances) {
        Vector3 offsetPos = {barrel.position.x, barrel.position.y + 20, barrel.position.z}; //move the barrel up a bit
        DrawModelEx(barrelModel, offsetPos, Vector3{0, 1, 0}, 0.0f, Vector3{0.5f, 0.5f, 0.5f}, barrel.tint); //scaled half size
    }
}




void DrawDungeonFloor(Model floorModel) {
    for (const FloorTile& tile : floorTiles) {
        DrawModelEx(floorModel, tile.position, Vector3{0,1,0}, 0.0f, Vector3{1,1,1}, tile.tint);
    }
}


void DrawDungeonWalls(Model wallModel) {
    const float wallHeight = 445.0f; // Match your actual wall height

    for (const WallInstance& wall : wallInstances) {
        // Draw base wall
        DrawModelEx(wallModel, wall.position, Vector3{0, 1, 0}, wall.rotationY, Vector3{1, 1, 1}, wall.tint);

        // Draw second stacked wall
        // Vector3 topPos = wall.position;
        // topPos.y += wallHeight;
        // DrawModelEx(wallModel, topPos, Vector3{0, 1, 0}, wall.rotationY, Vector3{1, 1, 1}, wall.tint);
    }
}

void DrawDungeonDoorways(Model archwayModel){

    for (const DoorwayInstance& d : doorways) {
        DrawModelEx(archwayModel, d.position, {0, 1, 0}, d.rotationY * RAD2DEG, {0.7f, 0.85f, 0.68f}, d.tint);
    
    }

    for (const Door& door : doors){
   
        DrawFlatDoor(door);
        //DrawBoundingBox(door.collider, RED);
        //DrawSphere(player.position, player.radius, GREEN);
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
            Vector3{1, 1, 1},
            ceiling.tint
        );
    }
}


// void DrawDungeonPillars(){
//     for (const PillarInstance& pillar : pillars){
//         DrawModelEx(lampModel, pillar.position, Vector3{0, 1, 0}, pillar.rotation, Vector3{1, 1, 1}, WHITE);
//     }
// }

void DrawDungeonPillars(float deltaTime, Camera3D camera) {
    for (size_t i = 0; i < pillars.size(); ++i) {
        const PillarInstance& pillar = pillars[i];
        Fire& fire = fires[i];

        // Draw the pedestal model
        DrawModelEx(lampModel, pillar.position, Vector3{0, 1, 0}, pillar.rotation, Vector3{1, 1, 1}, WHITE);

        // Animate fire
        fire.fireAnimTimer += deltaTime;
        if (fire.fireAnimTimer >= fire.fireFrameDuration) {
            fire.fireAnimTimer -= fire.fireFrameDuration;
            fire.fireFrame = (fire.fireFrame + 1) % 60;
        }

        // Calculate frame position in sheet
        int frameX = fire.fireFrame % 10;
        int frameY = fire.fireFrame / 10;
        Rectangle sourceRect = {
            (float)(frameX * 64),
            (float)(frameY * 64),
            64.0f,
            64.0f
        };

        // Position the fire above the bowl
        Vector3 firePos = pillar.position;
        firePos.y += 130; // Adjust this to match your pedestal height

        // Draw animated fire as billboard
        Vector2 fireSize = {100, 100};
        DrawBillboardRec(camera, fireSheet, sourceRect, firePos, fireSize, WHITE);
    }
}


void UpdateDoorwayTints(Vector3 playerPos) {
    const float playerLightRange = 1000.0f;
    const float minBrightness = 0.2f;

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
    const float playerLightRange = 1000.0f;
    const float minBrightness = 0.2f;

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

void ResolveBoxSphereCollision(const BoundingBox& box, Vector3& position, float radius) {
    // Clamp player position to the inside of the box
    float closestX = Clamp(position.x, box.min.x, box.max.x);
    float closestY = Clamp(position.y, box.min.y, box.max.y);
    float closestZ = Clamp(position.z, box.min.z, box.max.z);

    Vector3 closestPoint = { closestX, closestY, closestZ };
    Vector3 pushDir = Vector3Subtract(position, closestPoint);
    float distance = Vector3Length(pushDir);

    if (distance == 0.0f) {
        // If player is exactly on the box surface, push arbitrarily
        pushDir = {1.0f, 0.0f, 0.0f};
        distance = 0.001f;
    }

    float overlap = radius - distance;
    if (overlap > 0.0f) {
        Vector3 correction = Vector3Scale(Vector3Normalize(pushDir), overlap);
        position = Vector3Add(position, correction);
    }
}

BoundingBox MakeDoorBoundingBox(Vector3 position, float rotationY, float halfWidth, float height, float depth) {
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
    raptorPtrs.clear();
    raptors.clear();
    skeletons.clear();
    skeletonPtrs.clear();
    doors.clear();
    doorways.clear();
    collectables.clear();

}

