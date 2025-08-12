// render_pipeline.cpp
#include "render_pipeline.h"
#include "resourceManager.h"
#include "ui.h"
#include "world.h"
#include "transparentDraw.h"
#include "boat.h"
#include "rlgl.h"
#include "dungeonGeneration.h"
#include "player.h"
#include "input.h"
#include "camera_system.h"



void RenderFrame(Camera3D& camera, Player& player, float dt) {
    // --- 3D scene to sceneTexture ---
    BeginTextureMode(R.GetRenderTexture("sceneTexture"));
        ClearBackground(SKYBLUE);
        float farClip = isDungeon ? 10000.0f : 50000.0f;
        float nearclip = 60.0f;
        CameraSystem::Get().BeginCustom3D(camera, nearclip, farClip);

        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(R.GetModel("skyModel"), camera.position, 10000.0f, WHITE);
        rlEnableDepthMask(); rlEnableDepthTest();
        rlSetBlendMode(BLEND_ALPHA);

        if (!isDungeon) {
            DrawModel(terrainModel, {-terrainScale.x/2,0,-terrainScale.z/2}, 1.0f, WHITE);
            DrawModel(R.GetModel("waterModel"), {0, waterPos.y + (float)sin(GetTime()*0.9f)*0.9f, 0}, 1.0f, WHITE);
            DrawModel(R.GetModel("bottomPlane"), {0, waterHeightY - 100, 0}, 1.0f, DARKBLUE);
            DrawBoat(player_boat);
            BeginShaderMode(R.GetShader("cutoutShader"));
            DrawTrees(trees, R.GetModel("shadowQuad"), camera);
            EndShaderMode();
            DrawBushes(bushes, R.GetModel("shadowQuad"));
            DrawDungeonDoorways();
            
            DrawOverworldProps();
        } else {
            DrawDungeonFloor();
            DrawDungeonWalls();
            DrawDungeonCeiling();
            DrawDungeonBarrels();
            DrawDungeonChests();
            DrawDungeonPillars(dt);
            DrawDungeonDoorways();
        }

        DrawPlayer(player, camera);
        DrawBullets(camera);
        DrawBloodParticles(camera);
        DrawCollectableWeapons(player, dt);
        HandleWaves();
        // transparency last
        rlDisableDepthMask();
        DrawTransparentDrawRequests(camera);
        rlEnableDepthMask();

        EndBlendMode();
        EndMode3D();
        rlDisableDepthTest();
    EndTextureMode();

    // --- post to postProcessTexture ---
    BeginTextureMode(R.GetRenderTexture("postProcessTexture"));
        BeginShaderMode(R.GetShader("fogShader"));
            DrawTextureRec(R.GetRenderTexture("sceneTexture").texture,
                {0,0,(float)GetScreenWidth(),-(float)GetScreenHeight()}, {0,0}, WHITE);
        EndShaderMode();
    EndTextureMode();

    // --- final to backbuffer + UI ---
    BeginDrawing();
        ClearBackground(WHITE);
        BeginShaderMode(R.GetShader("bloomShader"));
            DrawTextureRec(R.GetRenderTexture("postProcessTexture").texture,
                {0,0,(float)GetScreenWidth(),-(float)GetScreenHeight()}, {0,0}, WHITE);
        EndShaderMode();

        
        if (pendingLevelIndex != -1) {
            DrawText("Loading...", GetScreenWidth()/2 - MeasureText("Loading...", 20)/2,
                     GetScreenHeight()/2, 20, WHITE);
        } else {
            DrawHealthBar(player);
            DrawStaminaBar(player);
            DrawManaBar(player);
            if (player.activeWeapon == WeaponType::MagicStaff) DrawMagicIcon();
            DrawText(TextFormat("Gold: %d", (int)player.displayedGold), 32, GetScreenHeight()-120, 30, GOLD);
           
            if (debugInfo) {
                DrawTimer(ElapsedTime);
                DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
                DrawText("PRESS TAB FOR FREE CAMERA", GetScreenWidth()/2 + 280, 30, 20, WHITE);
            }
            player.inventory.DrawInventoryUIWithIcons(itemTextures, slotOrder, 20, GetScreenHeight() - 80, 64);
        }
    EndDrawing();
}
