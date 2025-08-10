#include "raylib.h"
#include <iostream>
#include "raymath.h"
#include "rlgl.h"
#include "string"
#include "world.h"
#include "vegetation.h"
#include "player.h"
#include "input.h"
#include "boat.h"
#include "character.h"
#include "algorithm"
#include "sound_manager.h"
#include "level.h"
#include "dungeonGeneration.h"
#include "pathfinding.h"
#include "collectable.h"
#include "inventory.h"
#include "collisions.h"
#include "custom_rendertexture.h"
#include "transparentDraw.h"
#include <direct.h>
#include "collectableWeapon.h"
#include "ui.h"
#include "resourceManager.h"
#include "render_pipeline.h"

int main() { 
    InitWindow(1600, 900, "Marooned");
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

    // Camera 
    Camera3D camera = { 0 };
    camera.position = startPosition;
    camera.target = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;


    Vector3 terrainPosition = { //center the terrain around 0, 0, 0
            -terrainScale.x / 2.0f,
            0.0f,
            -terrainScale.z / 2.0f
    }; 
    
    
    //main game loop
    while (!WindowShouldClose()) {
        ElapsedTime += GetFrameTime();
        float deltaTime = GetFrameTime();
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
     
        UpdateInputMode(); //handle both gamepad and keyboard/mouse
        debugControls(camera); 

        R.UpdateShaders(camera);
        UpdateFade(deltaTime, camera); //triggers init level on fadeout
        UpdateEnemies(deltaTime);
        UpdateBullets(camera, deltaTime);
        lightBullets(deltaTime);
        UpdateDecals(deltaTime);
        UpdateMuzzleFlashes(deltaTime);
        UpdateBoat(player_boat, deltaTime);
        UpdateCollectables(camera, deltaTime); 
        UpdateDungeonChests();

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
        HandleWaves();
        HandleDoorInteraction(camera);
        if (isDungeon){
            //only handle lighting in dungeons. Oustide we don't touch the tint. 
            ApplyBakedLighting();
            HandleDungeonTints(deltaTime);
        }

        

        //gather up everything 2d and put it into a vector of struct drawRequests, then sort and draw every billboard/quad in the game.
        //Draw in order of furthest fisrt, closest last.  
        GatherTransparentDrawRequests(camera, deltaTime);
        DrawTransparentDrawRequests(camera);

            

        if (IsGamepadAvailable(0)) {  //free camera with gamepad
            UpdateCameraWithGamepad(camera);
        }

        HandleCameraPlayerToggle(camera, player, controlPlayer);
        UpdateCameraAndPlayer(camera, player, controlPlayer, deltaTime);


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
