#include "raylib.h"
#include <iostream>
#include "world.h"
#include "input.h"
#include "boat.h"
#include "sound_manager.h"
#include "dungeonGeneration.h"
#include "collisions.h"
#include "ui.h"
#include "resourceManager.h"
#include "render_pipeline.h"
#include "camera_system.h"
#include "lighting.h"

bool squareRes = true; // set true for 1024x1024, false for widescreen

int main() { 
    drawCeiling = false;
    int screenWidth = squareRes ? 1280 : 1600;
    int screenHeight = squareRes ? 1024 : 900;

    InitWindow(screenWidth, screenHeight, "Marooned");
    InitAudioDevice();
    SetTargetFPS(60);
    DisableCursor();
    SetExitKey(KEY_NULL); //Escape brings up menu, not quit
    ResourceManager::Get().LoadAllResources();
    ResourceManager::Get().SetShaderValues();

    SoundManager::GetInstance().LoadSounds();
    SoundManager::GetInstance().PlayMusic("dungeonAir");
    SoundManager::GetInstance().PlayMusic("jungleAmbience");
    SetMusicVolume(SoundManager::GetInstance().GetMusic("jungleAmbience"), 0.5f);

    controlPlayer = true; //start as player //hit tab for free camera
    float aspect = ((float)GetScreenWidth()/ (float)GetScreenHeight());

    float fovy = (aspect <= 1.0f) ? 55 : 45; //if the aspect ratio is square, bump up the FOV
    CameraSystem::Get().Init(startPosition);
    CameraSystem::Get().SetFOV(fovy);
    
    //main game loop
    while (!WindowShouldClose()) {
        ElapsedTime += GetFrameTime();
        float deltaTime = GetFrameTime();

       // Use the active camera everywhere:
        Camera3D& camera = CameraSystem::Get().Active();
        
        //Main Menu - level select 
        if (currentGameState == GameState::Menu) {
            UpdateMenu(camera);
            BeginDrawing();
            DrawMenu(selectedOption, levelIndex);
            EndDrawing();

            if (currentGameState == GameState::Quit) break;
            

            continue; // skip the rest of the game loop
        }



        if (IsKeyPressed(KEY_ESCAPE)) currentGameState = GameState::Menu;


        UpdateMusicStream(SoundManager::GetInstance().GetMusic(isDungeon ? "dungeonAir" : "jungleAmbience"));
     
        //update context
        UpdateFade(deltaTime, camera); //triggers init level on fadeout
        debugControls(camera, deltaTime); 
        R.UpdateShaders(camera);
        UpdateEnemies(deltaTime);
        UpdateBullets(camera, deltaTime);
        GatherFrameLights();
        EraseBullets();
        UpdateDecals(deltaTime);
        UpdateMuzzleFlashes(deltaTime);
        UpdateBoat(player_boat, deltaTime);
        UpdateCollectables(deltaTime); 
        UpdateLauncherTraps(deltaTime);
        UpdateDungeonChests();
        ApplyLavaDPS(player, deltaTime, 1);
        HandleWaves();

        //collisions
        CheckBulletHits(camera); //bullet collision
        TreeCollision(camera); //player and raptor vs tree
        WallCollision();
        DoorCollision();
        SpiderWebCollision();
        barrelCollision();
        ChestCollision();
        HandleEnemyPlayerCollision(&player);
        pillarCollision();
        HandleMeleeHitboxCollision(camera);

        HandleDoorInteraction(camera);

        if (isDungeon){
            //only handle lighting in dungeons. 
            HandleWeaponTints();
            HandleDungeonTints();
        }

        //gather up everything 2d and put it into a vector of struct drawRequests, then we sort and draw every billboard/quad in the game.
        GatherTransparentDrawRequests(camera, deltaTime);

        controlPlayer = CameraSystem::Get().IsPlayerMode();

        // Update camera based on player
        UpdateWorldFrame(deltaTime, player);
        UpdatePlayer(player, deltaTime, camera);
        
        if (!isLoadingLevel && isDungeon) BuildDynamicLightmapFromFrameLights(frameLights);

        RenderFrame(camera, player, deltaTime);
        
    }

    // Cleanup
    ClearLevel();
    ResourceManager::Get().UnloadAll();
    SoundManager::GetInstance().UnloadAll();
    CloseAudioDevice();
    CloseWindow();

    //system("pause"); // ← waits for keypress
    return 0;
}
