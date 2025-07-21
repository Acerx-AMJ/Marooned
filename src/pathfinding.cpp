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

    // Use a map from tile to where we came from //used to reconstruct the path
    std::unordered_map<int, Vector2> cameFrom;

    auto toIndex = [&](int x, int y) {
        return y * width + x;  //uniquely encodes (x, y) into a single number that is used as a key for the unordered map
    };

    cameFrom[toIndex((int)start.x, (int)start.y)] = {-1, -1}; //mark the start as having no parent

    // 4-way movement
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    while (!frontier.empty()) {
        Vector2 current = frontier.front();
        frontier.pop();

        if ((int)current.x == (int)goal.x && (int)current.y == (int)goal.y) {
            break; //reached the goal, return path. 
        }

        for (int i = 0; i < 4; ++i) {
            int nx = (int)current.x + dx[i];
            int ny = (int)current.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue; //out of bounds
            if (!walkable[nx][ny]) continue; //unwalkable

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

    // Trim final step so skeleton stops adjacent to player //they stop too far away
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

            if (c.a == 0) {
                walkable[x][y] = false;
                continue;
            }

            bool black   = (c.r == 0 && c.g == 0 && c.b == 0);       // walls
            bool blue    = (c.r == 0 && c.g == 0 && c.b == 255);     // barrels
            bool yellow  = (c.r == 255 && c.g == 255 && c.b == 0);   // light pedestals
            bool skyBlue = (c.r == 0 && c.g == 128 && c.b == 255);   // chests
            bool purple  = (c.r == 128 && c.g == 0 && c.b == 128);   // closed doors
            bool aqua    = (c.r == 0 && c.g == 255 && c.b == 255);   //Aqua

            walkable[x][y] = !(black || blue || yellow || skyBlue || purple || aqua);
        }
    }
}





Vector2 WorldToImageCoords(Vector3 worldPos) {
    int x = (int)(worldPos.x / tileSize);
    int y = (int)(worldPos.z / tileSize);

    // Flip Y and X to match how image pixels map to world
    x = dungeonWidth - 1 - x;
    y = dungeonHeight - 1 - y;

    return { (float)x, (float)y };
}


bool IsWalkable(int x, int y, const Image& dungeonMap) {
    if (x < 0 || x >= dungeonMap.width || y < 0 || y >= dungeonMap.height)
        return false;

    Color c = GetImageColor(dungeonMap, x, y);

    // Transparent = not walkable
    if (c.a == 0)
        return false;

    // Match walkability rules from ConvertImageToWalkableGrid
    bool black    = (c.r == 0 && c.g == 0 && c.b == 0);       // walls
    bool blue     = (c.r == 0 && c.g == 0 && c.b == 255);     // barrels
    bool yellow   = (c.r == 255 && c.g == 255 && c.b == 0);   // light pedestals
    bool skyBlue  = (c.r == 0 && c.g == 128 && c.b == 255);   // chests 
    bool purple   = (c.r == 128 && c.g == 0 && c.b == 128);   // closed doors
    bool aqua     = (c.r == 0 && c.g == 255 && c.b == 255);   //Aqua

    return !(black || blue || yellow || skyBlue || purple || aqua);
}



bool IsTileOccupied(int x, int y, const std::vector<Character*>& skeletons, const Character* self) {
    for (const Character* s : enemyPtrs) {
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



Vector2 GetRandomReachableTile(const Vector2& start, const Character* self, int maxAttempts) {
    int patrolRadius = 3;
    for (int i = 0; i < maxAttempts; ++i) {
        int rx = (int)start.x + GetRandomValue(-patrolRadius, patrolRadius);
        int ry = (int)start.y + GetRandomValue(-patrolRadius, patrolRadius);

        if (rx < 0 || ry < 0 || rx >= dungeonWidth || ry >= dungeonHeight)
            continue;

        if (!walkable[rx][ry]) continue;
        if (IsTileOccupied(rx, ry, enemyPtrs, self)) continue;

        Vector2 target = {(float)rx, (float)ry};
        if (!LineOfSightRaycast(start, target, dungeonImg, 100, 0.0f)) continue;

        return target;
    }

    // Return invalid coords if nothing found
    return {-1, -1};
}

bool TrySetRandomPatrolPath(const Vector2& start, Character* self, std::vector<Vector3>& outPath) {
    if (isLoadingLevel) return false;
    Vector2 randomTile = GetRandomReachableTile(start, self);

    if (IsWalkable(randomTile.x, randomTile.y, dungeonImg)) return false;

    std::vector<Vector2> tilePath = FindPath(start, randomTile);
    if (tilePath.empty()) return false;

    outPath.clear();
    for (const Vector2& tile : tilePath) {
        Vector3 worldPos = GetDungeonWorldPos(tile.x, tile.y, tileSize, dungeonPlayerHeight);
        worldPos.y += 80.0f; //skeles at 80
        if (self->type == CharacterType::Pirate) worldPos.y = 160;
        outPath.push_back(worldPos);
    }

    return !outPath.empty();
}


bool HasWorldLineOfSight(Vector3 from, Vector3 to, float epsilonFraction) {
    if (isLoadingLevel) return false;
    Ray ray = { from, Vector3Normalize(Vector3Subtract(to, from)) };
    float maxDistance = Vector3Distance(from, to);
    float epsilon = epsilonFraction * maxDistance;

    for (const WallRun& wall : wallRunColliders) {
        RayCollision hit = GetRayCollisionBox(ray, wall.bounds);
        if (hit.hit && hit.distance + epsilon < maxDistance) {
            return false;
        }
    }

    for (const Door& door: doors) {
        if (!door.isOpen){
            RayCollision hit = GetRayCollisionBox(ray, door.collider);
            
            if (hit.hit && hit.distance + epsilon < maxDistance) {
                return false;
            }
        }

    }


    return true;
}

Vector2 TileToWorldCenter(Vector2 tile) {
    return {
        tile.x + 0.5f * tileSize,
        tile.y + 0.5f * tileSize
    };
}




bool LineOfSightRaycast(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps, float epsilon) {
    if (isLoadingLevel) return false;
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

        
        if (SingleRayBlocked(offsetStart, end, dungeonMap, maxSteps, epsilon)) {
            blockedCount++;
            if (blockedCount > maxBlocked) return false;
        }
    }

    return true;
}

bool SingleRayBlocked(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps, float epsilon) {
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

        // Early out: if close enough to the end
        if (Vector2Distance((Vector2){x, y}, end) < epsilon) {
            return false;
        }

        Color c = GetImageColor(dungeonMap, tileX, tileY);

        if (c.r == 0 && c.g == 0 && c.b == 0) return true; // wall
        if (c.r == 255 && c.g == 255 && c.b == 0) return true; //light pedestal
        if (c.r == 128 && c.g == 0 && c.b == 128 && !IsDoorOpenAt(tileX, tileY)) return true; //closed door
        if (c.r == 0 && c.g == 255 && c.b == 255 && !IsDoorOpenAt(tileX, tileY)) return true; //locked closed door
        

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
            Vector2 start = TileToWorldCenter(path[i]);
            Vector2 end   = TileToWorldCenter(path[j]);

            if (LineOfSightRaycast(start, end, dungeonMap, 100, 0.0f)) {
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




