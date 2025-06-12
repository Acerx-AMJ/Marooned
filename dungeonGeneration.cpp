#include "dungeonGeneration.h"
#include "raymath.h"
#include "resources.h"
#include <iostream>
#include "player.h"
#include "vector"
#include "world.h"
//static std::vector<Vector3> floorTilePositions;
std::vector<FloorTile> floorTiles;

static std::vector<WallInstance> wallInstances;

std::vector<CeilingTile> ceilingTiles;
std::vector<BarrelInstance> barrelInstances;
std::vector<PillarInstance> pillars;
std::vector<WallRun> wallRunColliders;
std::vector<LightSource> dungeonLights;
std::vector<LightSource> bulletLights;

Image dungeonImg; //save the dungeon info globaly
Color* dungeonPixels = nullptr;
int dungeonWidth = 0;
int dungeonHeight = 0;

// === Constants ===
const Color COLOR_BLACK  = { 0, 0, 0, 255 };
const Color COLOR_BLUE   = { 0, 0, 255, 255 };


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

    max.y += height + 1000; // Add height

    return { min, max };
}


void LoadDungeonLayout(const std::string& imagePath) {
    if (dungeonPixels) {
        UnloadImageColors(dungeonPixels);
    }

    dungeonImg = LoadImage(imagePath.c_str());
    dungeonPixels = LoadImageColors(dungeonImg);
    dungeonWidth = dungeonImg.width;
    dungeonHeight = dungeonImg.height;
    

    
    //🟩 Green = Player Start

    //🔵 Blue = Barrel

    //🟥 Red = Raptor

    //🟨 Yellow = Light

    //🟪 Purple = Key
    
}

Vector3 FindSpawnPoint(Color* pixels, int width, int height, float tileSize, float baseY) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color current = pixels[y * width + x];
            if (current.r == 0 && current.g == 255 && current.b == 0) {
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
            Vector3 pos = GetDungeonWorldPos(x, y, tileSize, baseY);

            FloorTile tile;
            tile.position = pos;
            tile.tint = WHITE;

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



void GenerateWallTiles(float tileSize, float baseY) {
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

                    BoundingBox bounds = MakeWallBoundingBox(a, b, wallThickness, wallHeight);
                    wallRunColliders.push_back({ a, b, 0.0f, bounds });
                }
            }
        }
    }
}






void GenerateCeilingTiles(float ceilingOffsetY) {
    ceilingTiles.clear();

    for (const FloorTile& floor : floorTiles) {
        CeilingTile ceiling;
        ceiling.position = Vector3Add(floor.position, {0, ceilingOffsetY, 0});
        ceiling.tint = GRAY; // default tint

        ceilingTiles.push_back(ceiling);
    }
}

//const Color BARREL_COLOR = {0, 0, 255, 255};

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

void GenerateRaptorsFromImage(float tileSize, float baseY) {
    raptors.clear(); // Clear existing raptors if needed

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

    for (Character& r : raptors) {
        raptorPtrs.push_back(&r);
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
                pillars.push_back({pPos, 1.0f}); //put a pillar wherever there is a lightsource
            }
        }
    }
}

Vector3 GetDungeonWorldPos(int x, int y, float tileSize, float baseY) {
    int flippedY = dungeonHeight - 1 - y;
    int flippedX = dungeonWidth - 1 - x;

    return Vector3{
        flippedX * tileSize + tileSize / 2.0f,
        baseY,
        flippedY * tileSize + tileSize / 2.0f
    };
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
    const float wallHeight = 200.0f; // Match your actual wall height

    for (const WallInstance& wall : wallInstances) {
        // Draw base wall
        DrawModelEx(wallModel, wall.position, Vector3{0, 1, 0}, wall.rotationY, Vector3{1, 1, 1}, wall.tint);

        // Draw second stacked wall
        Vector3 topPos = wall.position;
        topPos.y += wallHeight;
        DrawModelEx(wallModel, topPos, Vector3{0, 1, 0}, wall.rotationY, Vector3{1, 1, 1}, wall.tint);
    }
}

void DrawDungeonPillars(Model pillarModel){
    for (const PillarInstance& pillar : pillars){
        DrawModelEx(pillarModel, pillar.position, Vector3{0, 1, 0}, pillar.rotation, Vector3{1, 1, 1}, WHITE);
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


void ClearDungeon() {
    //floorTilePositions.clear();
    floorTiles.clear();
    wallInstances.clear();
    //ceilingTilePositions.clear(); // if used
    ceilingTiles.clear();
}

