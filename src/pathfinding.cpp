#include "pathfinding.h"
#include <queue>
#include <unordered_map>
#include "raymath.h"
#include <algorithm>
#include "dungeonGeneration.h"
#include "world.h"
#include "character.h"

std::vector<std::vector<bool>> walkable;


std::vector<Vector2> FindPath(Vector2 start, Vector2 goal) {
    int width = walkable.size();
    int height = walkable[0].size();

    std::queue<Vector2> frontier;
    frontier.push(start);

    // Use a map from tile to where we came from
    std::unordered_map<int, Vector2> cameFrom;

    auto toIndex = [&](int x, int y) {
        return y * width + x; // unique key for unordered_map
    };

    cameFrom[toIndex((int)start.x, (int)start.y)] = {-1, -1};

    // 4-way movement
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    while (!frontier.empty()) {
        Vector2 current = frontier.front();
        frontier.pop();

        if ((int)current.x == (int)goal.x && (int)current.y == (int)goal.y) {
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = (int)current.x + dx[i];
            int ny = (int)current.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;
            if (!walkable[nx][ny]) continue;

            int index = toIndex(nx, ny);
            if (cameFrom.count(index) == 0) {
                frontier.push({(float)nx, (float)ny});
                cameFrom[index] = current;
            }
        }
    }

    // Reconstruct path
    std::vector<Vector2> path;
    Vector2 current = goal;

    while (!(current.x == -1 && current.y == -1)) {
        path.push_back(current);
        int index = toIndex((int)current.x, (int)current.y);
        if (cameFrom.count(index) == 0) break;
        current = cameFrom[index];
    }

    std::reverse(path.begin(), path.end());

    // Optional: check if path is valid
    if (path.size() == 1 && (int)path[0].x != (int)goal.x) {
        return {}; // goal unreachable
    }

    // Trim final step so skeleton stops adjacent to player
    // if (path.size() > 1) {
    //     path.pop_back();
    // }

    return path;
}


void ConvertImageToWalkableGrid(const Image& dungeonMap) {
    walkable.clear();
    walkable.resize(dungeonMap.width, std::vector<bool>(dungeonMap.height, false));

    for (int x = 0; x < dungeonMap.width; ++x) {
        for (int y = 0; y < dungeonMap.height; ++y) {
            Color c = GetImageColor(dungeonMap, x, y);

            // If pixel is fully transparent, mark it not walkable
            if (c.a == 0) {
                walkable[x][y] = false;
                continue;
            }

            bool isWall = (c.r < 50 && c.g < 50 && c.b < 50); // black
            bool isBarrel = (c.b > 200 && c.r < 100 && c.g < 100); // blue barrel

            walkable[x][y] = !(isWall || isBarrel);
        }
    }
}


// void ConvertImageToWalkableGrid(const Image& dungeonMap) {
//     walkable.clear();
//     walkable.resize(dungeonMap.width, std::vector<bool>(dungeonMap.height, false));

//     for (int x = 0; x < dungeonMap.width; ++x) {
//         for (int y = 0; y < dungeonMap.height; ++y) {
//             Color c = GetImageColor(dungeonMap, x, y);

//             // Define colors
//             bool isWall = (c.r < 50 && c.g < 50 && c.b < 50); // black
//             bool isBarrel = (c.b > 200 && c.r < 100 && c.g < 100); // blue barrel, skeletons will also navigate around barrels
            

//             walkable[x][y] = !(isWall || isBarrel);
//         }
//     }
// }

Vector2 WorldToImageCoords(Vector3 worldPos) {
    int x = (int)(worldPos.x / tileSize);
    int y = (int)(worldPos.z / tileSize);

    // Flip Y and X to match how image pixels map to world
    x = dungeonWidth - 1 - x;
    y = dungeonHeight - 1 - y;

    return { (float)x, (float)y };
}


bool IsWalkable(int x, int y) {
    if (x < 0 || x >= walkable.size() || y < 0 || y >= walkable[0].size()) return false;
    return walkable[x][y];
}

bool IsTileOccupied(int x, int y, const std::vector<Character*>& skeletons, const Character* self) {
    for (const Character* s : skeletons) {
        if (s == self || s->state == CharacterState::Death) continue; 

        Vector2 tile = WorldToImageCoords(s->position);
        if ((int)tile.x == x && (int)tile.y == y) {
            return true;
        }
    }
    return false;
}

Character* GetTileOccupier(int x, int y, const std::vector<Character*>& skeletons, const Character* self) {
    //skeles can't occupy the same tile while stoped. 
    for (Character* s : skeletons) {
        if (s == self || s->state == CharacterState::Death) continue;

        Vector2 tile = WorldToImageCoords(s->position);
        if ((int)tile.x == x && (int)tile.y == y) {
            return s; // first one we find
        }
    }
    return nullptr;
}

bool LineOfSightRaycast(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps) {
    const int numRays = 5;
    const float spread = 0.1f; // widen fan

    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    float distance = Vector2Length(Vector2Subtract(end, start));
    if (distance == 0) return true;

    Vector2 perp = { -dir.y, dir.x };

    int blockedCount = 0;
    const int maxBlocked = 3;

    for (int i = 0; i < numRays; ++i) {
        
        float offset = ((float)i - numRays / 2) * (spread / (float)numRays);
        Vector2 offsetStart = Vector2Add(start, Vector2Scale(perp, offset));

        
        if (SingleRayBlocked(offsetStart, end, dungeonMap, maxSteps)) {
            blockedCount++;
            if (blockedCount > maxBlocked) return false;
        }
    }

    return true;
}


bool SingleRayBlocked(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = sqrtf(dx*dx + dy*dy);
    if (distance == 0) return false;

    float stepX = dx / distance;
    float stepY = dy / distance;

    float x = start.x;
    float y = start.y;

    for (int i = 0; i < distance && i < maxSteps; ++i) {
        int tileX = (int)x;
        int tileY = (int)y;

        if (tileX < 0 || tileX >= dungeonMap.width || tileY < 0 || tileY >= dungeonMap.height)
            return true;

        Color c = GetImageColor(dungeonMap, tileX, tileY);

        if (c.r < 50 && c.g < 50 && c.b < 50) return true; // wall
        if (c.r == 128 && c.g == 0 && c.b == 128 && !IsDoorOpenAt(tileX, tileY)) return true; // closed door
        if (c.r == 0 && c.g == 255 && c.b == 255 && !IsDoorOpenAt(tileX, tileY)) return true; //locked door, 
        //skeletons can see through exit and entrance doors but it don't matter because how they are placed. 

        x += stepX;
        y += stepY;
    }

    return false;
}


std::vector<Vector2> SmoothPath(const std::vector<Vector2>& path, const Image& dungeonMap) {
    //check for short cuts. skip points if you can connect stright to a point further along. 
   
    std::vector<Vector2> result;

    if (path.empty()) return result;

    size_t i = 0;
    while (i < path.size()) {
        result.push_back(path[i]);

        size_t j = path.size() - 1;
        bool found = false;

        for (; j > i + 1; --j) {
            if (LineOfSightRaycast(path[i], path[j], dungeonMap, 100)) {
                found = true;
                break;
            }
        }

        if (found) {
            i = j;  // jump to further straight-line segment
        } else {
            ++i;    // fallback: step forward to prevent infinite loop
        }
    }

    return result;
}

