#include "dungeonGeneration.h"
#include "raymath.h"
static std::vector<Vector3> floorTilePositions;
static std::vector<std::pair<Vector3, float>> wallInstances; // position + Y rotation
//static std::vector<WallInstance> wallInstances;

static std::vector<Vector3> ceilingTilePositions;

static Image dungeonImg;
static Color* dungeonPixels = nullptr;
static int dungeonWidth = 0;
static int dungeonHeight = 0;

void LoadDungeonLayout(const std::string& imagePath) {
    if (dungeonPixels) {
        UnloadImageColors(dungeonPixels);
    }

    dungeonImg = LoadImage(imagePath.c_str());
    dungeonPixels = LoadImageColors(dungeonImg);
    dungeonWidth = dungeonImg.width;
    dungeonHeight = dungeonImg.height;
}

void GenerateFloorTiles(float tileSize, float baseY) {
    floorTilePositions.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Vector3 pos = {
                x * tileSize,
                baseY,
                y * tileSize
            };
            floorTilePositions.push_back(pos);
        }
    }
}

void GenerateWallTiles(float tileSize, float baseY) {
    wallInstances.clear();

    for (int y = 0; y < dungeonHeight; y++) {
        for (int x = 0; x < dungeonWidth; x++) {
            Color pixel = dungeonPixels[y * dungeonWidth + x];
            if (pixel.r < 50 && pixel.g < 50 && pixel.b < 50) {
                bool rightWhite = (x < dungeonWidth - 1) && dungeonPixels[y * dungeonWidth + (x + 1)].r > 200;
                bool leftWhite  = (x > 0) && dungeonPixels[y * dungeonWidth + (x - 1)].r > 200;
                bool upWhite    = (y > 0) && dungeonPixels[(y - 1) * dungeonWidth + x].r > 200;
                bool downWhite  = (y < dungeonHeight - 1) && dungeonPixels[(y + 1) * dungeonWidth + x].r > 200;

                float rotationY = 0.0f;
                bool horizontal = (leftWhite || rightWhite);
                bool vertical = (upWhite || downWhite);

                if (vertical && !horizontal) rotationY = 90.0f;
                else if (horizontal && !vertical) rotationY = 0.0f;
                else if (vertical && horizontal) rotationY = 0.0f;
                else rotationY = 0.0f;

                Vector3 pos = {
                    x * tileSize,
                    baseY,
                    y * tileSize
                };

                pos.x = roundf(pos.x / tileSize) * tileSize;
                pos.z = roundf(pos.z / tileSize) * tileSize;        

                wallInstances.emplace_back(pos, rotationY);
            }
        }
    }
}

void GenerateCeilingTiles(float ceilingOffsetY) {
    ceilingTilePositions.clear();

    for (const Vector3& pos : floorTilePositions) {
        Vector3 ceilingPos = {
            pos.x,
            pos.y + ceilingOffsetY,
            pos.z
        };
        ceilingTilePositions.push_back(ceilingPos);
    }
}


void DrawDungeonFloor(Model floorTileModel) {
    for (const Vector3& pos : floorTilePositions) {
        DrawModel(floorTileModel, pos, 1.0f, WHITE);
    }
}

void DrawDungeonWalls(Model wallModel) {
    for (const auto& [pos, rot] : wallInstances) {
        // Draw base wall
        DrawModelEx(wallModel, pos, Vector3{0,1,0}, rot, Vector3{1,1,1}, WHITE);

        // Draw stacked wall above
        Vector3 topPos = pos;
        topPos.y += 200;
        DrawModelEx(wallModel, topPos, Vector3{0,1,0}, rot, Vector3{1,1,1}, WHITE);
    }
}


void DrawDungeonCeiling(Model ceilingTileModel, float ceilingOffsetY) {
    for (const Vector3& pos : floorTilePositions) {
        Vector3 ceilingPos = {
            pos.x,
            pos.y + ceilingOffsetY,
            pos.z
        };
        DrawModelEx(
            ceilingTileModel,
            ceilingPos,                  // Position of the tile
            Vector3{1, 0, 0},            // Rotation axis (X-axis)
            180.0f,                      // Rotation angle
            Vector3{1, 1, 1},            // Scale
            GRAY
        );

    }
}

