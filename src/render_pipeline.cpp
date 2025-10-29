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
#include "camera_system.h"

void RenderFrame(Camera3D& camera, Player& player, float dt) {
    Shader& terrainShader = ResourceManager::Get().GetShader("terrainShader");
    //int locShadow      = GetShaderLocation(terrainShader, "u_ShadowMask");
    // --- 3D scene to sceneTexture ---
    BeginTextureMode(ResourceManager::Get().GetRenderTexture("sceneTexture"));
        ClearBackground(SKYBLUE);
        float farClip = isDungeon ? 10000.0f : 50000.0f;
        float nearclip = 30.0f;
        CameraSystem::Get().BeginCustom3D(camera, nearclip, farClip);

        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(ResourceManager::Get().GetModel("skyModel"), camera.position, 10000.0f, WHITE);
        rlEnableDepthMask(); rlEnableDepthTest();
        rlSetBlendMode(BLEND_ALPHA);

        if (!isDungeon) {

            DrawModel(terrainModel, {-terrainScale.x/2,0,-terrainScale.z/2}, 1.0f, WHITE);

            DrawModel(ResourceManager::Get().GetModel("waterModel"), {0, waterPos.y + (float)sin(GetTime()*0.9f)*0.9f, 0}, 1.0f, WHITE);
            DrawModel(ResourceManager::Get().GetModel("bottomPlane"), {0, waterHeightY - 100, 0}, 1.0f, DARKBLUE);
            DrawBoat(player_boat);
            BeginShaderMode(ResourceManager::Get().GetShader("cutoutShader"));
            DrawTrees(trees, camera); 
            DrawBushes(bushes); //alpha cuttout bushes as well as tree leaf
            EndShaderMode();
            DrawDungeonDoorways();          
            DrawOverworldProps();
        } else {

            DrawDungeonFloor();
            DrawDungeonWalls();
            DrawDungeonDoorways();
            DrawDungeonCeiling();

            DrawDungeonBarrels();
            DrawLaunchers();
            DrawDungeonChests();
            DrawDungeonPillars();
            // for (WallRun& b : wallRunColliders){ //debug draw wall colliders
            //     DrawBoundingBox(b.bounds, WHITE);
            // }

        }

        DrawPlayer(player, camera);
        
        DrawEnemyShadows();
        DrawBullets(camera);
        DrawCollectableWeapons(player, dt);
        HandleWaves();
        // transparency last

        DrawTransparentDrawRequests(camera);
        rlDisableDepthMask();
        DrawBloodParticles(camera);
        rlEnableDepthMask();


        EndBlendMode();
        EndMode3D();
        rlDisableDepthTest();
    EndTextureMode();

    // --- post to postProcessTexture ---
    BeginTextureMode(ResourceManager::Get().GetRenderTexture("postProcessTexture"));
    {
        BeginShaderMode(ResourceManager::Get().GetShader("fogShader"));
            auto& sceneRT = ResourceManager::Get().GetRenderTexture("sceneTexture");
            Rectangle src = { 0, 0,
                            (float)sceneRT.texture.width,
                            -(float)sceneRT.texture.height }; // flip Y!
            Rectangle dst = { 0, 0,
                            (float)GetScreenWidth(),
                            (float)GetScreenHeight() };
            DrawTexturePro(sceneRT.texture, src, dst, {0,0}, 0.0f, WHITE);
        EndShaderMode();
    }
    EndTextureMode();

    // --- final to backbuffer + UI ---
    BeginDrawing();
        ClearBackground(WHITE);
        BeginShaderMode(ResourceManager::Get().GetShader("bloomShader"));
            auto& postRT = ResourceManager::Get().GetRenderTexture("postProcessTexture");
            Rectangle src = { 0, 0,
                            (float)postRT.texture.width,
                            -(float)postRT.texture.height }; // flip Y!
            Rectangle dst = { 0, 0,
                            (float)GetScreenWidth(),
                            (float)GetScreenHeight() };
            DrawTexturePro(postRT.texture, src, dst, {0,0}, 0.0f, WHITE);
        EndShaderMode();

        
        if (pendingLevelIndex != -1) {
            DrawText("Loading...", GetScreenWidth()/2 - MeasureText("Loading...", 20)/2,
                     GetScreenHeight()/2, 20, WHITE);
        } else {
            //health mana stam bars
            if (controlPlayer){
                DrawHUDBars(player);
                if (player.activeWeapon == WeaponType::MagicStaff) DrawMagicIcon();
                DrawText(TextFormat("Gold: %d", (int)player.displayedGold), 32, GetScreenHeight()-120, 30, GOLD);
                player.inventory.DrawInventoryUIWithIcons(itemTextures, slotOrder, 20, GetScreenHeight() - 80, 64);
                DrawHints();

            } 

            if (debugInfo) {
                DrawTimer(ElapsedTime);
                DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
                DrawText("PRESS TAB FOR FREE CAMERA", GetScreenWidth()/2, 30, 20, WHITE);


            }
            
        }
    EndDrawing();
}
