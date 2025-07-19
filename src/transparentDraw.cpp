#include "transparentDraw.h"
#include "raylib.h"
#include "character.h"
#include "decal.h"
#include "dungeonGeneration.h"
#include "raymath.h"
#include "world.h"
#include "resources.h"
#include "vector"
#include "rlgl.h"
#include "algorithm"

std::vector<BillboardDrawRequest> billboardRequests;

float GetAdjustedBillboardSize(float baseSize, float distance) {
    //billboard scalling is way to extreme because of the size of the world. compensate by enlarging it a tiny bit depending on distance. 
    const float compensationFactor = 0.0001f;

    // Boost size as distance grows
    return baseSize * (1.0f + distance * compensationFactor);
}


void GatherEnemies(Camera& camera) {
    //all billboards in the game including decals and spider webs are drawn in a single draw pass. first we need to gather all the billboards
    //and decals and flat quads. Then we sort them by distance to camera and draw them in that order. This prevents textures occluding eachother. 
    for (Character* enemy : enemyPtrs) {
        if (enemy->isDead && enemy->deathTimer <= 0.0f) continue;

        float dist = Vector3Distance(camera.position, enemy->position);

        // Frame source rectangle
        Rectangle sourceRect = {
            (float)(enemy->currentFrame * enemy->frameWidth),
            (float)(enemy->rowIndex * enemy->frameHeight),
            (float)enemy->frameWidth,
            (float)enemy->frameHeight
        };

        // Slight camera-facing offset to avoid z-fighting
        Vector3 camDir = Vector3Normalize(Vector3Subtract(camera.position, enemy->position));
        Vector3 offsetPos = Vector3Add(enemy->position, Vector3Scale(camDir, 10.0f));
       
       
        //compensate for extreme billboard scaling, things shrink at a distance but not by much. 
        float billboardSize = GetAdjustedBillboardSize(enemy->frameWidth * enemy->scale, dist);
        // Dynamic tint for damage
        Color finalTint = (enemy->hitTimer > 0.0f) ? (Color){255, 50, 50, 255} : WHITE;

        billboardRequests.push_back({
            Billboard_FacingCamera,
            offsetPos,
            enemy->texture,
            sourceRect,
            billboardSize,
            finalTint,
            dist,
            0.0f
        });


    }
}

void GatherCollectables(Camera& camera, const std::vector<Collectable>& collectables) {
    for (const Collectable& c : collectables) {
        float dist = Vector3Distance(camera.position, c.position);

        billboardRequests.push_back({
            Billboard_FacingCamera,
            c.position,
            c.icon,
            Rectangle{0, 0, (float)c.icon->width, (float)c.icon->height},
            c.scale,
            WHITE,
            dist,
            0.0f
        });
    }
}



void GatherDungeonFires(Camera& camera, float deltaTime) {
    for (size_t i = 0; i < pillars.size(); ++i) {
        PillarInstance& pillar = pillars[i];
        Fire& fire = fires[i];

        // Animate fire
        fire.fireAnimTimer += deltaTime;
        if (fire.fireAnimTimer >= fire.fireFrameDuration) {
            fire.fireAnimTimer -= fire.fireFrameDuration;
            fire.fireFrame = (fire.fireFrame + 1) % 60;
        }

        // Compute source rect
        int frameX = fire.fireFrame % 10;
        int frameY = fire.fireFrame / 10;
        Rectangle sourceRect = {
            (float)(frameX * 64),
            (float)(frameY * 64),
            64.0f,
            64.0f
        };

        // Compute fire position
        Vector3 firePos = pillar.position;
        firePos.y += 130;

        // Add to billboard requests
        float dist = Vector3Distance(camera.position, firePos);
        billboardRequests.push_back({
            Billboard_FacingCamera,
            firePos,
            &fireSheet,
            sourceRect,
            100.0f,
            WHITE,
            dist,
            0.0f
        });
    }
}

void GatherWebs(Camera& camera) {
    for (const SpiderWebInstance& web : spiderWebs) {
        //if (web.destroyed && !web.showBrokeWebTexture) continue;

        Texture2D* tex = web.destroyed ? &brokeWebTexture : &spiderWebTexture;

        billboardRequests.push_back({
            Billboard_FixedFlat,
            web.position,
            tex,
            Rectangle{0, 0, (float)spiderWebTexture.width, (float)spiderWebTexture.height},
            300.0f,
            WHITE,
            Vector3Distance(camera.position, web.position),
            web.rotationY
        });
    }
}

void GatherDecals(Camera& camera, const std::vector<Decal>& decals) {
    for (const Decal& decal : decals) {
        if (!decal.alive) continue;

        float dist = Vector3Distance(camera.position, decal.position);

        Rectangle sourceRect;

        if (decal.type == DecalType::Explosion) {
            sourceRect = {
                static_cast<float>(decal.currentFrame * 196),
                0,
                196,
                190
            };
        } else {
            sourceRect = {
                static_cast<float>(decal.currentFrame * 64),
                0,
                64,
                64
            };
        }

        billboardRequests.push_back({
            Billboard_Decal,
            decal.position,
            decal.texture,
            sourceRect,
            decal.size,
            WHITE,
            dist,
            0.0f
        });
    }
}


void GatherMuzzleFlashes(Camera3D camera, Weapon& weapon) {
    if (weapon.flashTimer <= 0.0f) return;

    float dist = Vector3Distance(camera.position, weapon.muzzlePos);

    billboardRequests.push_back({
        Billboard_MuzzleFlash,
        weapon.muzzlePos,
        &weapon.muzzleFlashTexture,
        Rectangle{0, 0, (float)weapon.muzzleFlashTexture.width, (float)weapon.muzzleFlashTexture.height},
        weapon.flashSize,         // or weapon.flashSize if dynamic
        WHITE,
        dist,
        0.0f
    });
}

// void GatherRevolverMuzzleFlashes(Camera3D camera, Revolver& revolver) {
//     if (revolver.flashTimer <= 0.0f) return;

//     float dist = Vector3Distance(camera.position, revolver.muzzlePos);

//     billboardRequests.push_back({
//         Billboard_MuzzleFlash,
//         revolver.muzzlePos,
//         &revolver.muzzleFlashTexture,
//         Rectangle{0, 0, (float)revolver.muzzleFlashTexture.width, (float)revolver.muzzleFlashTexture.height},
//         revolver.flashSize,         // or weapon.flashSize if dynamic
//         WHITE,
//         dist,
//         0.0f
//     });
// }



void GatherTransparentDrawRequests(Camera& camera, Weapon& weapon, float deltaTime) {
    billboardRequests.clear();

    GatherEnemies(camera);
    GatherDungeonFires(camera, deltaTime);
    GatherWebs(camera);
    GatherDecals(camera, decals);
    GatherMuzzleFlashes(camera, weapon);

    GatherCollectables(camera, collectables);
}

void DrawTransparentDrawRequests(Camera& camera) {
    std::sort(billboardRequests.begin(), billboardRequests.end(),
        [](const BillboardDrawRequest& a, const BillboardDrawRequest& b) {
            return a.distanceToCamera > b.distanceToCamera;
        });

    for (const BillboardDrawRequest& req : billboardRequests) {
        rlDisableDepthMask();
        
        switch (req.type) {
            case Billboard_FacingCamera: //use draw billboard for both decals, and enemies. 
            case Billboard_Decal:
                DrawBillboardRec(
                    camera,
                    *(req.texture),
                    req.sourceRect,
                    req.position,
                    Vector2{req.size, req.size},
                    req.tint
                );
                break;
            case Billboard_FixedFlat: //special case for webs
                DrawFlatWeb(
                    *(req.texture),
                    req.position,
                    req.size,
                    req.size,
                    req.rotationY,
                    req.tint
                );
                break;
        }

        rlEnableDepthMask();
    }
}
