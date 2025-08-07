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

        //Main Menu - level select 
        if (currentGameState == GameState::Menu) {
            UpdateMenu(camera);
            BeginDrawing();
            DrawMenu(selectedOption, levelIndex);
            EndDrawing();

            if (currentGameState == GameState::Quit) break;
            

            continue; // skip the rest of the game loop
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentGameState = GameState::Menu;
        }
        float deltaTime = GetFrameTime();
        if (isDungeon) {
            UpdateMusicStream(SoundManager::GetInstance().GetMusic("dungeonAir"));
        } else {
            UpdateMusicStream(SoundManager::GetInstance().GetMusic("jungleAmbience"));
        }
        UpdateInputMode(); //handle both gamepad and keyboard/mouse
        debugControls(camera); 
        R.UpdateShaders(camera);
        sortTrees(camera); //sort trees by distance to camera
        
        UpdateFade(deltaTime, camera); //triggers init level on fadeout
        UpdateEnemies(deltaTime);
        UpdateBullets(camera, deltaTime);
        UpdateDecals(deltaTime);
        UpdateMuzzleFlashes(deltaTime);
        UpdateBoat(player_boat, deltaTime);
        UpdateCollectables(camera, deltaTime); 
        UpdateDungeonChests();

        lightBullets(deltaTime);
        CheckBulletHits(camera); //bullet collision
        TreeCollision(camera); //player and raptor vs tree
        WallCollision();
        DoorCollision();
        SpiderWebCollision();
        barrelCollision();
        ChestCollision();
        HandleEnemyPlayerCollision(&player);
        //HandleEnemyEnemyCollision(); //enemies don't collide with each other
        pillarCollision();
        HandleMeleeHitboxCollision(camera);
        HandleDoorInteraction(camera);
        ApplyBakedLighting();
        //gather up everything 2d and put it into a vector of struct drawRequests, then sort and draw every billboard/quad in the game.
        //Draw in order of furthest fisrt, closest last.  
        GatherTransparentDrawRequests(camera, deltaTime);
        DrawTransparentDrawRequests(camera);

       
        if (isDungeon){
            HandleDungeonTints(deltaTime);
            
        }

        if (IsGamepadAvailable(0)) {  //free camera with gamepad
            UpdateCameraWithGamepad(camera);
        }

        HandleCameraPlayerToggle(camera, player, controlPlayer);
        UpdateCameraAndPlayer(camera, player, controlPlayer, deltaTime);

        //water
        float wave = sin(GetTime() * 0.9f) * 0.9f;  // slow, subtle vertical motion
        float animatedWaterLevel = waterHeightY + wave;
        Vector3 waterPos = {0, animatedWaterLevel, 0};
        bottomPos = {0, waterHeightY - 100, 0};

        // === RENDER TO TEXTURE ===
        BeginTextureMode(R.GetRenderTexture("sceneTexture"));
        ClearBackground(SKYBLUE);
        float farClip = isDungeon ? 10000.0f : 50000.0f;//10k in dungeons 50k outside

        BeginCustom3D(camera, farClip);
        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(R.GetModel("skyModel"), camera.position, 10000.0f, WHITE); //draw skybox with no depthmask or test or backface culling, leave backfaceculling off. 
        rlEnableDepthMask(); rlEnableDepthTest();
        rlSetBlendMode(BLEND_ALPHA); //required 
        //rlEnableColorBlend(); //not sure

        DrawDungeonFloor();
        DrawDungeonWalls();
        DrawDungeonCeiling();
        DrawDungeonBarrels();
        DrawDungeonChests();
        DrawDungeonPillars(deltaTime, camera);
        DrawDungeonDoorways();

        DrawPlayer(player, camera);
        DrawBullets(camera); 
        DrawBloodParticles(camera);
        DrawCollectableWeapons(player, deltaTime);

        //draw things with transparecy last.
        rlDisableDepthMask();
        DrawTransparentDrawRequests(camera);


        rlEnableDepthMask();
        
        if (!isDungeon) { //not a dungeon, draw terrain. 
            DrawModel(terrainModel, terrainPosition, 1.0f, WHITE);
            DrawModel(R.GetModel("waterModel"), waterPos, 1.0f, WHITE); 
            DrawModel(R.GetModel("bottomPlane"), bottomPos, 1.0f, DARKBLUE); //a second plane below water plane. to prevent seeing through the world when looking down.
            DrawBoat(player_boat);
            DrawTrees(trees, R.GetModel("shadowQuad")); 
            DrawBushes(bushes, R.GetModel("shadowQuad"));
        }


        EndBlendMode();
        EndMode3D(); //////////////////EndMode3

        rlDisableDepthTest();
        EndTextureMode();//////end drawing to texture

                // === POSTPROCESS TO postProcessTexture ===
        BeginTextureMode(R.GetRenderTexture("postProcessTexture"));
            BeginShaderMode(R.GetShader("fogShader")); // original post shader
                DrawTextureRec(R.GetRenderTexture("sceneTexture").texture, 
                    (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                    (Vector2){0, 0}, WHITE);
            EndShaderMode();
        EndTextureMode(); // postProcessTexture now contains processed scene

        // === Final draw to screen ===
        BeginDrawing();
            ClearBackground(WHITE);

            BeginShaderMode(R.GetShader("bloomShader")); // second pass
                DrawTextureRec(R.GetRenderTexture("postProcessTexture").texture,
                    (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                    (Vector2){0, 0}, WHITE);
            EndShaderMode();

        ///2D on top of render texture
        if (pendingLevelIndex != -1) { //fading out...draw loading text and nothing else. 
            DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20) / 2, GetScreenHeight() / 2, 20, WHITE);
        }else{
            DrawHealthBar(player);
            DrawStaminaBar(player);
            DrawManaBar(player);
            if (player.activeWeapon == WeaponType::MagicStaff) DrawMagicIcon();
            DrawText(TextFormat("Gold: %d", (int)player.displayedGold), 32, GetScreenHeight()-120, 30, GOLD); 
            if (debugInfo){//press ~ to hide debug info
                DrawTimer(ElapsedTime); 
                DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
                DrawText(currentInputMode == InputMode::Gamepad ? "Gamepad" : "Keyboard", 10, 30, 20, LIGHTGRAY);
                DrawText("PRESS TAB FOR FREE CAMERA", GetScreenWidth()/2 + 280, 30, 20, WHITE);

            }

            player.inventory.DrawInventoryUIWithIcons(itemTextures, slotOrder, 20, GetScreenHeight() - 80, 64);
            
        }

        EndDrawing();

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
