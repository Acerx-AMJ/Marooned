#include "dungeonGeneration.h"
#include "raymath.h"
#include "resources.h"
//static std::vector<Vector3> floorTilePositions;
std::vector<FloorTile> floorTiles;
//static std::vector<std::pair<Vector3, float>> wallInstances; // position + Y rotation
static std::vector<WallInstance> wallInstances;
//static std::vector<Vector3> ceilingTilePositions;
std::vector<CeilingTile> ceilingTiles;


std::vector<WallRun> wallRunColliders;

static Image dungeonImg;
static Color* dungeonPixels = nullptr;
static int dungeonWidth = 0;
static int dungeonHeight = 0;



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
}

// void GenerateFloorTiles(float tileSize, float baseY) {
//     floorTilePositions.clear();
//     //fill the dungeon area with floor tiles, ignore the walls for now. 
//     for (int y = 0; y < dungeonHeight; y++) {
//         for (int x = 0; x < dungeonWidth; x++) {
//             Vector3 pos = {
//                 x * tileSize,
//                 baseY,
//                 y * tileSize
//             };
//             floorTilePositions.push_back(pos);
//         }
//     }


// }

void GenerateFloorTiles(float tileSize, float baseY) {
    floorTiles.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Vector3 pos = {
                x * tileSize,
                baseY,
                y * tileSize
            };

            // Default tint: white (full brightness)
            FloorTile tile;
            tile.position = pos;
            tile.tint = WHITE;

            floorTiles.push_back(tile);
        }
    }
}


void GenerateWallTiles(float tileSize, float baseY) {
    //place wall tiles on top of floor tiles intersecting them instead of cutting out space for walls. 
    //generate bounding boxes from wallrun start to end. 
    wallInstances.clear();
    wallRunColliders.clear();
    
    float wallThickness = 50.0f;
    float wallHeight = 200.0f;

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color current = dungeonPixels[y * dungeonWidth + x];
            if (current.r < 50 && current.g < 50 && current.b < 50) { //black

                // Horizontal pair
                if (x < dungeonWidth - 1) {
                    Color right = dungeonPixels[y * dungeonWidth + (x + 1)];
                    if (right.r < 50) {
                        float midX = (x + 0.5f) * tileSize;
                        float z = y * tileSize;

                        Vector3 pos = {midX, baseY, z};
                        //wallInstances.emplace_back(pos, 90.0f);
                        WallInstance wall;
                        wall.position = pos;
                        wall.rotationY = 90.0f;
                        //wall.bounds = ComputeBoundingBox(pos, rot); // Optional if you want bounds now
                        wall.tint = WHITE; // Or a starting color

                        wallInstances.push_back(wall);


                        Vector3 start = { x * tileSize, baseY, z };
                        Vector3 end = { (x + 1) * tileSize, baseY, z };
                        BoundingBox bounds = MakeWallBoundingBox(start, end, wallThickness, wallHeight);
                        wallRunColliders.push_back({start, end, 90.0f, bounds});
                    }
                }

                // Vertical pair
                if (y < dungeonHeight - 1) {
                    Color down = dungeonPixels[(y + 1) * dungeonWidth + x];
                    if (down.r < 50) {
                        float xPos = x * tileSize;
                        float midZ = (y + 0.5f) * tileSize;

                        Vector3 pos = {xPos, baseY, midZ};
                        //wallInstances.emplace_back(pos, 0.0f);
                        WallInstance wall;
                        wall.position = pos;
                        wall.rotationY = 0.0f;
                        //wall.bounds = ComputeBoundingBox(pos, rot); // Optional if you want bounds now
                        wall.tint = WHITE; // Or a starting color

                        wallInstances.push_back(wall);


                        Vector3 start = { xPos, baseY, y * tileSize };
                        Vector3 end = { xPos, baseY, (y + 1) * tileSize };
                        BoundingBox bounds = MakeWallBoundingBox(start, end, wallThickness, wallHeight);
                        wallRunColliders.push_back({start, end, 0.0f, bounds});
                    }
                }
            }
        }
    }
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


void GenerateCeilingTiles(float ceilingOffsetY) {
    ceilingTiles.clear();

    for (const FloorTile& floor : floorTiles) {
        CeilingTile ceiling;
        ceiling.position = Vector3Add(floor.position, {0, ceilingOffsetY, 0});
        ceiling.tint = GRAY; // default tint

        ceilingTiles.push_back(ceiling);
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

void UpdateWallTints(Vector3 playerPos) {
    const float maxLightDistance = 6000.0f;
    const float minBrightness = 0.2f;

    Vector3 warmColor = {1.0f, 0.85f, 0.7f};

    for (WallInstance& wall : wallInstances) {
        float dist = Vector3Distance(playerPos, wall.position);
        float brightness = Clamp(1.0f - (dist / maxLightDistance), minBrightness, 1.0f);

        Vector3 tinted = Vector3Scale(warmColor, brightness);
        wall.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}


void UpdateFloorTints(Vector3 playerPos) {
    const float maxLightDistance = 6000.0f;
    const float minBrightness = 0.2f;

    Vector3 warmColor = {1.0f, 0.85f, 0.7f};

    for (FloorTile& tile : floorTiles) {
        float dist = Vector3Distance(playerPos, tile.position);
        float brightness = Clamp(1.0f - (dist / maxLightDistance), minBrightness, 1.0f);
        Vector3 tinted = Vector3Scale(warmColor, brightness);
        tile.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}


void UpdateCeilingTints(Vector3 playerPos) {
    const float maxLightDistance = 8000.0f;
    const float minBrightness = 0.2f;

    Vector3 warmCeilingColor = {0.8f, 0.7f, 0.6f}; // slightly muted warm tone

    for (CeilingTile& ceiling : ceilingTiles) {
        float dist = Vector3Distance(playerPos, ceiling.position);
        float brightness = Clamp(1.0f - (dist / maxLightDistance), minBrightness, 1.0f);
        Vector3 tinted = Vector3Scale(warmCeilingColor, brightness);
        ceiling.tint = ColorFromNormalized((Vector4){ tinted.x, tinted.y, tinted.z, 1.0f });
    }
}




// void DrawDungeonWalls(Model wallModel) {

//     for (const auto& [pos, rot] : wallInstances) {

//         Matrix transform = MatrixIdentity();
//         transform = MatrixMultiply(transform, MatrixTranslate(pos.x, pos.y, pos.z));
//         transform = MatrixMultiply(transform, MatrixRotateY(DEG2RAD * rot)); // if rotated
//         transform = MatrixMultiply(transform, MatrixScale(1.0f, 1.0f, 1.0f)); // if needed

//         SetShaderValueMatrix(dungeonWallShader, GetShaderLocation(dungeonWallShader, "model"), transform);
//         // Draw base wall
//         DrawModelEx(wallModel, pos, Vector3{0,1,0}, rot, Vector3{1,1,1}, WHITE);

//         // Draw stacked wall above
//         Vector3 topPos = pos;
//         topPos.y += 200;
//         DrawModelEx(wallModel, topPos, Vector3{0,1,0}, rot, Vector3{1,1,1}, WHITE);
//     }
// }


// void DrawDungeonCeiling(Model ceilingTileModel, float ceilingOffsetY) {
//     for (const Vector3& pos : floorTilePositions) {
//         Vector3 ceilingPos = {
//             pos.x,
//             pos.y + ceilingOffsetY,
//             pos.z
//         };
//         DrawModelEx(
//             ceilingTileModel,
//             ceilingPos,                  // Position of the tile
//             Vector3{1, 0, 0},            // Rotation axis (X-axis)
//             180.0f,                      // Rotation angle
//             Vector3{1, 1, 1},            // Scale
//             GRAY
//         );

//     }
// }
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


void ClearDungeon() {
    //floorTilePositions.clear();
    floorTiles.clear();
    wallInstances.clear();
    //ceilingTilePositions.clear(); // if used
    ceilingTiles.clear();
}

