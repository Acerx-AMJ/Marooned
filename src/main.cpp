#include "render/lighting.h"
#include "render/render_pipeline.h"
#include "tools/boat.h"
#include "util/camera_system.h"
#include "util/collisions.h"
#include "util/resourceManager.h"
#include "util/sound_manager.h"
#include "util/ui.h"
#include "world/world.h"

bool squareRes = false; // set true for 1280x1024, false for widescreen
//TODO: make 1280 res work. How? 

int main() { 
    int screenWidth = squareRes ? 1280 : 1600;
    int screenHeight = squareRes ? 1024 : 900;

    drawCeiling = true; //debug no ceiling mode. drawCeiling is set by levelData so we can have some dungeons with and without ceilings. 

    InitWindow(screenWidth, screenHeight, "Marooned");
    //ToggleFullscreen(); //start full screen, toggle out to 1600x900
    //isFullscreen = true;
    InitAudioDevice();
    SetTargetFPS(60);
    DisableCursor();
    SetExitKey(KEY_NULL); //Escape brings up menu, not quit
    ResourceManager::Get().LoadAllResources();
    SoundManager::Get().LoadSounds();
    
    SoundManager::Get().PlayMusic("dungeonAir");
    SoundManager::Get().PlayMusic("jungleAmbience");
    SetMusicVolume(SoundManager::Get().GetMusic("jungleAmbience"), 0.5f);

    float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
    float fovy   = (aspect < (16.0f/9.0f)) ? 50.0f : 45.0f; //bump up FOV if it's narrower than 16x9

    CameraSystem::Get().Init(startPosition);
    CameraSystem::Get().SetFOV(fovy);

    
    //main game loop
    while (!WindowShouldClose()) {
        ElapsedTime += GetFrameTime();
        float deltaTime = GetFrameTime();
        
       // Use the active camera everywhere:
        Camera3D& camera = CameraSystem::Get().Active();
        
        UpdateFade(); //always update fade

        //Main Menu - level select 
        if (currentGameState == GameState::Menu) {
            UpdateMusicStream(SoundManager::Get().GetMusic("jungleAmbience"));
            if (switchFromMenu){ //HACK//// make lighting work on level load from door. When game state is menu, only menu code runs,
            //enabling us to cleanly switch levels and lightmaps. 
               
                InitLevel(levels[pendingLevelIndex], camera);
                pendingLevelIndex = -1;
                
                switchFromMenu = false;
                currentGameState = GameState::Playing;
            } 

            UpdateMenu(camera);
      
            //dont draw menu when doing the menu switching hack
            BeginDrawing();
            DrawMenu(selectedOption, levelIndex);
            EndDrawing();


            if (currentGameState == GameState::Quit) break;
            

            continue; // skip the rest of the game loop
        }

        if (IsKeyPressed(KEY_ESCAPE) && currentGameState != GameState::Menu) currentGameState = GameState::Menu;
        UpdateMusicStream(SoundManager::Get().GetMusic(isDungeon ? "dungeonAir" : "jungleAmbience"));

        //update context

        ResourceManager::Get().UpdateShaders(camera);
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
        ApplyLavaDPS(player, deltaTime, 10);
        HandleWaves();
        UpdateHintManager(deltaTime);
        
        //collisions
        UpdateCollisions(camera);

        HandleDoorInteraction();
        HandleWeaponTints();
        if (isDungeon){
            
            HandleDungeonTints();
        }

        //gather up everything 2d and put it into a vector of struct drawRequests, then we sort and draw every billboard/quad in the game.
        GatherTransparentDrawRequests(camera, deltaTime);

        // Update camera based on player
        UpdateWorldFrame(deltaTime, player);
        UpdatePlayer(player, deltaTime, camera);
        
        if (!isLoadingLevel && isDungeon) BuildDynamicLightmapFromFrameLights(frameLights);

        RenderFrame(camera, player, deltaTime); //draw everything
        
    }

    // Cleanup
    ClearLevel();
    ResourceManager::Get().UnloadAll();
    SoundManager::Get().UnloadAll();
    CloseAudioDevice();
    CloseWindow();
}
