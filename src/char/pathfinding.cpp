#include "char/pathfinding.h"

#include <algorithm>
#include <unordered_map>
#include <queue>
#include "raymath.h"
#include "char/character.h"
#include "util/utilities.h"
#include "world/world.h"

std::vector<std::vector<bool>> walkable; //grid of bools that mark walkabe/unwalkable tiles. 

std::vector<Vector2> FindPath(Vector2 start, Vector2 goal) {
    const int width  = (int)walkable.size();         // X dimension (cols)
    if (width == 0) return {};
    const int height = (int)walkable[0].size();      // Y dimension (rows)

    auto inBounds = [&](int x, int y){
        return x >= 0 && y >= 0 && x < width && y < height;
    };
    auto toIndex = [&](int x, int y){
        return y * width + x; // stride by width because first index is X
    };

    const int sx = (int)start.x, sy = (int)start.y;
    const int gx = (int)goal.x,  gy = (int)goal.y;

    if (!inBounds(sx, sy) || !inBounds(gx, gy)) return {};
    // If you require start/goal to be walkable, keep these:
    if (!walkable[sx][sy]) return {};
    if (!walkable[gx][gy]) return {};

    std::queue<Vector2> frontier;
    frontier.push({(float)sx, (float)sy});

    std::unordered_map<int, Vector2> cameFrom;
    cameFrom[toIndex(sx, sy)] = {-1, -1};

    static const int dx[4] = { 1, -1,  0,  0 };
    static const int dy[4] = { 0,  0,  1, -1 };

    bool reached = false;

    while (!frontier.empty()) {
        Vector2 cur = frontier.front(); frontier.pop();
        const int cx = (int)cur.x, cy = (int)cur.y;

        if (cx == gx && cy == gy) { reached = true; break; }

        for (int i = 0; i < 4; ++i) {
            const int nx = cx + dx[i];
            const int ny = cy + dy[i];
            if (!inBounds(nx, ny)) continue;
            if (!walkable[nx][ny]) continue; // NOTE: [x][y] on purpose

            const int idx = toIndex(nx, ny);
            if (cameFrom.count(idx) == 0) {
                cameFrom[idx] = cur;
                frontier.push({ (float)nx, (float)ny });
            }
        }
    }

    //if goal never discovered, no path.
    if (!reached) return {};

    // Reconstruct
    std::vector<Vector2> path;
    Vector2 cur = { (float)gx, (float)gy };
    while (!(cur.x == -1 && cur.y == -1)) {
        path.push_back(cur);
        cur = cameFrom[toIndex((int)cur.x, (int)cur.y)];
    }
    std::reverse(path.begin(), path.end());

    // Optional: stop one tile short (melee)
    // if (path.size() > 1) path.pop_back();

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
            bool aqua    = (c.r == 0 && c.g == 255 && c.b == 255);   // locked doors
            bool lava     = (c.r == 200 && c.g == 0 && c.b == 0);    // lava pit

            walkable[x][y] = !(black || blue || yellow || skyBlue || purple || aqua || lava);
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
    bool aqua     = (c.r == 0 && c.g == 255 && c.b == 255);   // locked doors
    bool lava     = (c.r == 200 && c.g == 0 && c.b == 0);     // lava

    return !(black || blue || yellow || skyBlue || purple || aqua || lava);
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

    if (!IsWalkable(randomTile.x, randomTile.y, dungeonImg)) return false;

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

//TODO: Create a second worldLOSLeaky

bool HasWorldLineOfSight(Vector3 from, Vector3 to, float epsilonFraction, LOSMode mode)
{
    Ray ray = { from, Vector3Normalize(Vector3Subtract(to, from)) };
    float maxDistance = Vector3Distance(from, to);
    float epsilon = epsilonFraction * maxDistance;

    // Walls always block
    for (const WallRun& wall : wallRunColliders) {
        RayCollision hit = GetRayCollisionBox(ray, wall.bounds);
        if (hit.hit && hit.distance + epsilon < maxDistance) return false;
    }

    if (mode == LOSMode::AI) {
        // For AI vision, a CLOSED door panel blocks. (Open doors do not.)
        for (const Door& door : doors) {
            if (!door.isOpen) {
                RayCollision hit = GetRayCollisionBox(ray, door.collider);
                if (hit.hit && hit.distance + epsilon < maxDistance) return false;
            }
        }
        // Optionally also block by door jambs/side colliders for AI (feels more "real"):
        for (const DoorwayInstance& dw : doorways) {
            for (const BoundingBox& sc : dw.sideColliders) {
                RayCollision hit = GetRayCollisionBox(ray, sc);
                if (hit.hit && hit.distance + epsilon < maxDistance) return false;
            }
        }
    } else { // LOSMode::Lighting
        // For lighting, let light leak through the middle of the doorway but block the sides.
        for (const DoorwayInstance& dw : doorways) {
            for (const BoundingBox& sc : dw.sideColliders) {
                RayCollision hit = GetRayCollisionBox(ray, sc);
                if (hit.hit && hit.distance + epsilon < maxDistance) return false;
            }
        }
        // NOTE: We intentionally do NOT test door panels here, so closed doors don't kill the glow.
    }

    return true;
}


// bool HasWorldLineOfSight(Vector3 from, Vector3 to, float epsilonFraction) {  

//     Ray ray = { from, Vector3Normalize(Vector3Subtract(to, from)) };
//     float maxDistance = Vector3Distance(from, to);
//     float epsilon = epsilonFraction * maxDistance;

//     for (const WallRun& wall : wallRunColliders) {
//         RayCollision hit = GetRayCollisionBox(ray, wall.bounds); //ray stops at the collider, epsilon pushes it further to the actual wall position. 
//         if (hit.hit && hit.distance + epsilon < maxDistance) {
//             return false;
//         }
//     }

//     // for (const Door& door: doors) {
//     //     if (!door.isOpen){
//     //         RayCollision hit = GetRayCollisionBox(ray, door.collider);
            
//     //         if (hit.hit && hit.distance + epsilon < maxDistance) {
//     //             return false;
//     //         }
//     //     }

//     // }

//     for (const DoorwayInstance& dw : doorways){ //check against side colliders not door way, let light leak through doors. 
//         for (const auto& sc : dw.sideColliders){
//             RayCollision hit = GetRayCollisionBox(ray, sc);

//             if (hit.hit && hit.distance + epsilon < maxDistance) {
//                 return false;
//             }
//         }
//     }


//     return true;
// }

Vector2 TileToWorldCenter(Vector2 tile) {
    return {
        tile.x + 0.5f * tileSize,
        tile.y + 0.5f * tileSize
    };
}


bool LineOfSightRaycast(Vector2 start, Vector2 end, const Image& dungeonMap, int maxSteps, float epsilon) {
    //raymarch the PNG map, do we use this? 
    const int numRays = 5;
    const float spread = 0.1f; // widen fan

    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    float distance = Vector2Length(Vector2Subtract(end, start));
    if (distance == 0) return true; 

    Vector2 perp = { -dir.y, dir.x };

    int blockedCount = 0;
    const int maxBlocked = 3;

    for (int i = 0; i < numRays; ++i) {
        
        float offset = ((float)i - (float)numRays / 2) * (spread / (float)numRays);
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

// Tunables
static constexpr int   kMaxLookahead = 8;      // tiles/waypoints ahead to try
static constexpr float kMaxSkipDist  = 12.0f;  // meters (or your world units)
static constexpr float kEpsFraction  = 0.01f;  // 1% of segment length

std::vector<Vector3> SmoothWorldPath(const std::vector<Vector3>& worldPath)
{
    std::vector<Vector3> out;
    if (worldPath.empty()) return out;

    size_t i = 0;
    const size_t N = worldPath.size();

    while (i < N) {
        // Always keep the current point
        out.push_back(worldPath[i]);

        // Try to jump ahead greedily
        size_t furthest = i;
        const size_t maxJ = std::min(N - 1, i + static_cast<size_t>(kMaxLookahead));

        for (size_t j = maxJ; j > i + 0; --j) {
            const float dist = Vector3Distance(worldPath[i], worldPath[j]);
            if (dist > kMaxSkipDist) continue;

            if (HasWorldLineOfSight(worldPath[i], worldPath[j], kEpsFraction)) {
                furthest = j;
                break; // take the longest valid skip we found
            }
        }

        if (furthest == i) {
            // Couldn’t skip—advance one to avoid infinite loop
            ++i;
        } else {
            // Jump to the furthest visible point
            i = furthest;
        }
    }

    // Ensure destination is included (in case the loop ended on a mid point)
    if (out.back().x != worldPath.back().x ||
        out.back().y != worldPath.back().y ||
        out.back().z != worldPath.back().z) {
        out.push_back(worldPath.back());
    }

    return out;
}


// std::vector<Vector2> SmoothPath(const std::vector<Vector2>& path, const Image& dungeonMap) {
//     //check for short cuts. skip points if you can connect stright to a point further along. 
   
//     std::vector<Vector2> result;

//     if (path.empty()) return result;
//     size_t i = 0;
//     while (i < path.size()) {
//         result.push_back(path[i]);

//         size_t j = path.size() - 1;
//         bool found = false;

//         for (; j > i + 1; --j) {
//             Vector2 start = TileToWorldCenter(path[i]);
//             Vector2 end   = TileToWorldCenter(path[j]);
            
            
//             if (LineOfSightRaycast(start, end, dungeonMap, 100, 0.0f)) { //raymarch the pixel map
//                 found = true;
//                 break;
//             }
//         }

//         if (found) {
//             i = j;  // jump to further straight-line segment
//         } else {
//             ++i;    // fallback: step forward to prevent infinite loop
//         }
//     }

//     return result;
// }

//Raptor steering behavior
Vector3 SeekXZ(const Vector3& pos, const Vector3& target, float maxSpeed) {
    // Fast “point at it”
    Vector3 dir = NormalizeXZ(Vector3Subtract(target, pos));
    return Vector3Scale(dir, maxSpeed);
}

// Ease in within slowRadius
Vector3 ArriveXZ(const Vector3& pos, const Vector3& target,
                        float maxSpeed, float slowRadius)
{
    Vector3 toT = Vector3Subtract(target, pos); toT.y = 0.0f;
    float d = sqrtf(toT.x*toT.x + toT.z*toT.z);
    if (d < 1e-4f) return {0,0,0};
    float t = fminf(1.0f, d / slowRadius);             // 0..1
    float speed = fmaxf(0.0f, maxSpeed * t);           // ramp down near target
    return Vector3Scale({toT.x/d, 0.0f, toT.z/d}, speed);
}

Vector3 FleeXZ(const Vector3& pos, const Vector3& threat, float maxSpeed) {
    Vector3 dir = NormalizeXZ(Vector3Subtract(pos, threat));
    return Vector3Scale(dir, maxSpeed);
}

// orbitRadius: desired ring distance from target
// clockwise: +1 for CW, -1 for CCW
// tangentGain: how strongly to circle (0..1 usually)
// radialGain: how strongly to correct toward the ring
Vector3 OrbitXZ(const Vector3& pos, const Vector3& target,
                       float orbitRadius, int clockwise,
                       float tangentGain, float radialGain,
                       float maxSpeed)
{
    // Radial from raptor -> target
    Vector3 r = Vector3Subtract(target, pos); r.y = 0.0f;
    float d = sqrtf(r.x*r.x + r.z*r.z);
    if (d < 1e-4f) return {0,0,0};
    Vector3 rN = { r.x/d, 0.0f, r.z/d };

    // Tangent: rotate radial ±90° in XZ
    Vector3 tN = (clockwise >= 0) ? Vector3{ rN.z, 0.0f, -rN.x }
                                  : Vector3{ -rN.z, 0.0f,  rN.x };

    // Radial correction pushes toward the ring (out if too close, in if too far)
    float radialErr = (d - orbitRadius);           // positive if too far
    Vector3 radialVel = Vector3Scale(rN, -radialErr * radialGain); // pull toward ring

    // Tangential circling velocity
    Vector3 tangentialVel = Vector3Scale(tN, tangentGain * maxSpeed);

    // Blend and clamp
    Vector3 v = Vector3Add(tangentialVel, radialVel);
    return Limit(v, maxSpeed);
}

// Keep a wanderAngle per-raptor; call each frame.
Vector3 WanderXZ(float& wanderAngle, float wanderTurnRate, float wanderSpeed, float dt) {
    // nudge angle
    wanderAngle += ((float)GetRandomValue(-1000,1000) / 1000.0f) * wanderTurnRate * dt;
    float s = sinf(wanderAngle), c = cosf(wanderAngle);
    return { s * wanderSpeed, 0.0f, c * wanderSpeed };
}

// returns true if movement was blocked by water this frame
bool StopAtWaterEdge(const Vector3& pos,
                            Vector3& desiredVel,     // in/out
                            float waterLevel,
                            float dt)
{
    // if we’re not moving, nothing to do
    float v2 = desiredVel.x*desiredVel.x + desiredVel.z*desiredVel.z;
    if (v2 < 1e-4f) return false;

    // Look a short distance ahead in the movement direction (XZ only)
    Vector3 dir = { desiredVel.x, 0.0f, desiredVel.z };
    float  speed = sqrtf(v2);
    float  look  = fmaxf(100.0f, speed * 0.4f); // small peek ahead; tweak

    Vector3 ahead = { pos.x + dir.x / speed * look,
                      pos.y,
                      pos.z + dir.z / speed * look };

    // Sample terrain height at the ahead point
    float hAhead = GetHeightAtWorldPosition(ahead, heightmap, terrainScale);

    if (hAhead <= waterLevel) {
        // hard stop at the shoreline
        desiredVel.x = desiredVel.z = 0.0f;
        return true;
    }
    return false;
}

bool IsWaterAtXZ(float x, float z, float waterLevel) {
    Vector3 waterPos = {x, waterLevel, z};
    return GetHeightAtWorldPosition(waterPos, heightmap, terrainScale) <= waterLevel;
}





