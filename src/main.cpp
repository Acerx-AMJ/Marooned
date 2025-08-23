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

int main() { 
    InitWindow(1024, 1024, "Marooned");
    InitAudioDevice();
    SetTargetFPS(60);
    ResourceManager::Get().LoadAllResources();
    ResourceManager::Get().SetShaderValues();
    SoundManager::GetInstance().LoadSounds();
    SoundManager::GetInstance().PlayMusic("dungeonAir");
    SoundManager::GetInstance().PlayMusic("jungleAmbience");
    SetMusicVolume(SoundManager::GetInstance().GetMusic("jungleAmbience"), 0.5f);
    DisableCursor();
    SetExitKey(KEY_NULL); //Escape brings up menu, not quit

    controlPlayer = true; //start as player //hit tab for free camera
    float aspect = ((float)GetScreenWidth()/ (float)GetScreenHeight());

    float fovy = (aspect <= 1.0f) ? 55 : 45; //is the aspect ratio is square, bump up the FOV
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
        debugControls(camera); 
        R.UpdateShaders(camera);
        UpdateFade(deltaTime, camera); //triggers init level on fadeout
        UpdateEnemies(deltaTime);
        UpdateBullets(camera, deltaTime);
        lightBullets(deltaTime);
        UpdateDecals(deltaTime);
        UpdateMuzzleFlashes(deltaTime);
        UpdateBoat(player_boat, deltaTime);
        UpdateCollectables(deltaTime); 
        UpdateDungeonChests();
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
            //only handle lighting in dungeons. Oustide we don't touch the tint. 
            ApplyBakedLighting();
            HandleDungeonTints();
        }

        //gather up everything 2d and put it into a vector of struct drawRequests, then we sort and draw every billboard/quad in the game.
        GatherTransparentDrawRequests(camera, deltaTime);

        controlPlayer = CameraSystem::Get().IsPlayerMode();

        // Update camera based on player
        UpdateWorldFrame(deltaTime, player);
        UpdatePlayer(player, deltaTime, camera);
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
