#include "raylib.h"
#include <iostream>
#include "raymath.h"
#include "rlgl.h"
#include "string"
#include "world.h"
#include "vegetation.h"
#include "player.h"
#include "resources.h"
#include "input.h"
#include "boat.h"
#include "character.h"
#include "algorithm"
#include "bullet.h"
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


void BeginCustom3D(Camera3D camera, float farClip) {
    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    float nearClip = 60.0f; //20 wider than the capsule. to 50k outside, 10k in dungeons

    float testFOV = 45.0f; // 45 is default 
    Matrix proj = MatrixPerspective(DEG2RAD * camera.fovy, (float)GetScreenWidth() / (float)GetScreenHeight(), nearClip, farClip);

    rlMultMatrixf(MatrixToFloat(proj));
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(view));
}

void ClearLevel() {
    billboardRequests.clear();
    removeAllCharacters();\
    activeBullets.clear();
    ClearDungeon();
    bulletLights.clear();
    dungeonEntrances.clear();

    RemoveAllVegetation();

    if (terrainMesh.vertexCount > 0) UnloadMesh(terrainMesh); //unload mesh when switching levels. 
    if (heightmap.data != nullptr) UnloadImage(heightmap); 

    isDungeon = false;
}

void GenerateEntrances(){
    for (const DungeonEntrance& e : dungeonEntrances) {
        Door d;
        d.position = e.position;
        d.rotationY = 0.0f; 
        d.doorTexture = &doorTexture;
        d.isOpen = false;
        d.isLocked = false;
        d.scale = {300, 365, 1};
        d.tint = WHITE;
        d.linkedLevelIndex = e.linkedLevelIndex; //use entrance's linkedLevelIndex to determine which dungeon to enter from overworld
        //allowing more than one enterance to dungeon per overworld

        d.doorType = DoorType::GoToNext; //Go to Dungeon

        float halfWidth = 200.0f;   // Half of the 400-unit wide doorway
        float height = 365.0f;
        float depth = 20.0f;        // Thickness into the doorway (forward axis)

        d.collider = MakeDoorBoundingBox(d.position, d.rotationY, halfWidth, height, depth); //the whole archway is covered by collider

        doors.push_back(d);

        DoorwayInstance dw;
        dw.position = e.position;
        dw.rotationY = 90.0f * DEG2RAD; //rotate to match door 0 rotation, we could rotate door to match arch instead.
        dw.isOpen = false;
        dw.isLocked = false;
        dw.tint = GRAY;

        doorways.push_back(dw);
    }

}


void InitLevel(const LevelData& level, Camera camera) {
    isLoadingLevel = true;
    //Called when starting game and changing level. init the level you pass it. the level is chosen by menu or door's linkedLevelIndex. 
    ClearLevel();//clears everything. 
    camera.position = player.position; //start as player, not freecam.
    levelIndex = level.levelIndex; //update current level index to new level. 

    //why not level.isDungeon? cause it crashes, isDungeon is false to begin and then set by level. Does that mean we create terrain mesh even for dungeons?
    //because if isDungeon is false this code runs. everything is set to zero or null anyway. do we init boat on dungeon levels? WE DO! 
    //just try putting if (level.isDungeon) first. I think something in dugneon still tries to touch the heightmap or maybe it's boat. 

    if (!isDungeon){ // Generate the terrain mesh and model from the heightmap
        vignetteStrengthValue = 0.2f; //less of vignette outdoors. 
        // Load and format the heightmap image
        heightmap = LoadImage(level.heightmapPath.c_str());
        ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
        
        terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
        terrainModel = LoadModelFromMesh(terrainMesh);
        terrainModel.materials[0].shader = terrainShader;
        dungeonEntrances = level.entrances; //get level entrances from level data

        generateRaptors(level.raptorCount, level.raptorSpawnCenter, 6000);
        GenerateEntrances();
        generateVegetation(); 
        InitBoat(player_boat, boatPosition);
    }

   
    if (level.isDungeon){
        isDungeon = true;
        vignetteStrengthValue = 0.5f; //darker vignette in dungeons
        //swordModel.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = swordClean; //start with a clean sword
        LoadDungeonLayout(level.dungeonPath);
        ConvertImageToWalkableGrid(dungeonImg);
        GenerateFloorTiles(floorHeight);//80
        GenerateWallTiles(wallHeight); //model is 400 tall with origin at it's center, so wallHeight is floorHeight + model height/2. 270
        GenerateCeilingTiles(ceilingHeight);//400
        GenerateBarrels(floorHeight);
        GenerateSpiderWebs(floorHeight);
        GenerateChests(floorHeight);
        GeneratePotions(floorHeight);
        GenerateKeys(floorHeight);
        GenerateWeapons(200);
        GenerateLightSources(floorHeight);
        GenerateDoorways(floorHeight, levelIndex); //calls generate doors from archways

        GenerateSkeletonsFromImage(dungeonEnemyHeight); //165
        GeneratePiratesFromImage(dungeonEnemyHeight);
        GenerateSpiderFromImage(dungeonEnemyHeight);


        if (levelIndex == 4){
            levels[0].startPosition = {-5653, 200, 6073}; //exit dungeon 3 to dungeon enterance 2 position. 
        }

      
    }
    
    Vector3 resolvedSpawn = level.startPosition; // default fallback
    if (first){
        resolvedSpawn = {5475.0f, 300.0f, -5665.0f}; //overriding start position with first level spwan point
        //first = false; first is set to false in player. after another thing needs to happen first time only.
    }
    

    if (isDungeon){
        Vector3 pixelSpawn = FindSpawnPoint(dungeonPixels, dungeonWidth, dungeonHeight, tileSize, floorHeight);
        if (pixelSpawn.x != 0 || pixelSpawn.z != 0) {
            resolvedSpawn = pixelSpawn;//Overriding start position with green pixel
            
        } 

    }
    isLoadingLevel = false;
    ResetAllBakedTints();
    BakeStaticLighting();
    InitPlayer(player, resolvedSpawn); //start at green pixel if there is one. otherwise level.startPos or first startPos

    //start with blunderbus and sword in that order
    player.collectedWeapons = {WeaponType::Blunderbuss, WeaponType::Sword};
    player.activeWeapon = WeaponType::Blunderbuss;
    player.currentWeaponIndex = 0;

}

void SpawnRaptor(Vector3 pos) {
    Character raptor(pos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);
    enemies.push_back(raptor);
    enemyPtrs.push_back(&enemies.back());
}



void UpdateEnemies(float deltaTime) {
    if (isLoadingLevel) return;
    for (Character& e : enemies){
        e.Update(deltaTime, player, heightmap, terrainScale);
    }
}

void UpdateMuzzleFlashes(float deltaTime) {
    for (auto& flash : activeMuzzleFlashes) {
        flash.age += deltaTime;
    }

    // Remove expired flashes
    activeMuzzleFlashes.erase(
        std::remove_if(activeMuzzleFlashes.begin(), activeMuzzleFlashes.end(),
                       [](const MuzzleFlash& flash) { return flash.age >= flash.lifetime; }),
        activeMuzzleFlashes.end());
}


void UpdateBullets(Camera& camera, float deltaTime) {
    for (Bullet& b : activeBullets) {
        b.Update(camera, deltaTime);
    }

    activeBullets.erase( //erase dead bullets. 
        std::remove_if(activeBullets.begin(), activeBullets.end(),
                    [](const Bullet& b) { return !b.IsAlive(); }),
        activeBullets.end());

}




void DrawBullets(Camera& camera) {
    for (const Bullet& b : activeBullets) {
        if (b.IsAlive()){
             b.Draw(camera);
        }
    }

}

void UpdateCollectables(Camera& camera, float deltaTime) { 
    for (int i = 0; i < collectables.size(); i++) {
        collectables[i].Update(deltaTime);

        // Pickup logic
        if (collectables[i].CheckPickup(player.position, 180.0f)) { //180 radius
            if (collectables[i].type == CollectableType::HealthPotion) {
                player.inventory.AddItem("HealthPotion");
                SoundManager::GetInstance().Play("clink");
            }
            else if (collectables[i].type == CollectableType::Key) {
                player.inventory.AddItem("GoldKey");
                SoundManager::GetInstance().Play("key");
            }
            else if (collectables[i].type == CollectableType::Gold) {
                player.gold += collectables[i].value;
                SoundManager::GetInstance().Play("key");
            } else if (collectables[i].type == CollectableType::ManaPotion) {
                player.inventory.AddItem("ManaPotion");
                SoundManager::GetInstance().Play("clink");
            }

            collectables.erase(collectables.begin() + i);
            i--;
        }
    }
}




void UpdateDecals(float deltaTime){
    for (auto& d : decals) {
        d.Update(deltaTime);
        
    }
    decals.erase(std::remove_if(decals.begin(), decals.end(),
                [](const Decal& d) { return !d.alive; }),
                decals.end());
}

void lightBullets(float deltaTime){
    //Fireball/iceball light

   // === Clean up expired bullet lights ===
    for (int i = bulletLights.size() - 1; i >= 0; --i) {
        bulletLights[i].age += deltaTime;
        if (bulletLights[i].age >= bulletLights[i].lifeTime) {
            bulletLights.erase(bulletLights.begin() + i);
        }
    }

   // === Collect new lights to add after cleanup ===
    std::vector<LightSource> newBulletLights;
    for (const Bullet& bullet : activeBullets) {
        if (bullet.type == BulletType::Default) continue;
        LightSource bulletLight;
        bulletLight.position = bullet.GetPosition();
        bulletLight.range = 300.0f;
        //bulletLight.intensity = 1.2f;
        bulletLight.lifeTime = 0.5f;
        bulletLight.age = 0.0f;
        if (bullet.type == BulletType::Iceball) {
            bulletLight.colorTint = {0.0f, 0.7f, 0.9f};
            bulletLight.type = LightType::Iceball;
        } else if (bullet.type == BulletType::Fireball) {
            bulletLight.colorTint = {1.0f, 0.15f, 0.0f};
            bulletLight.type = LightType::Fireball;
        } else {
            bulletLight.type = LightType::Other;
        }

        newBulletLights.push_back(bulletLight);
    }

    //=== Now safely append them ===
    bulletLights.insert(bulletLights.end(), newBulletLights.begin(), newBulletLights.end());
}

void UpdateFade(float deltaTime, Camera& camera){
    //fades out on death, and level transition if pendingLevelIndex != -1
    if (isFading) {
        if (fadeIn) {
            fadeToBlack += fadeSpeed * deltaTime;
            if (fadeToBlack >= 1.0f) {
                fadeToBlack = 1.0f;
                isFading = false;

                if (pendingLevelIndex != -1) {
                    InitLevel(levels[pendingLevelIndex], camera); //Start new Level
                    pendingLevelIndex = -1;

                    // Start fading back in
                    fadeIn = false;
                    isFading = true;
                }
            }
        } else {
            fadeToBlack -= fadeSpeed * deltaTime;
            if (fadeToBlack <= 0.0f) {
                fadeToBlack = 0.0f;
                isFading = false;
            }
        }

        SetShaderValue(fogShader, GetShaderLocation(fogShader, "fadeToBlack"), &fadeToBlack, SHADER_UNIFORM_FLOAT);
    }


}


void HandleDungeon(float deltaTime) {
    if (isLoadingLevel) return;
    WallCollision();

     //Vertex Color Lighting
    // === Update tints with updated light list ===

    UpdateWallTints(player.position);
    UpdateCeilingTints(player.position);
    UpdateFloorTints(player.position);
    UpdateBarrelTints(player.position);
    UpdateChestTints(player.position);
    UpdateDoorwayTints(player.position);
    UpdateDoorTints(player.position);
}

void DrawBloodParticles(Camera& camera){
    for (Character* enemy : enemyPtrs) { //draw enemy blood, blood is 3d so draw before billboards. 
            enemy->bloodEmitter.Draw(camera);
    }
}








int main() { 
    InitWindow(1600, 900, "Marooned");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadAllResources();
    DisableCursor();
    SetExitKey(KEY_NULL); //Escape brings up menu, not quit
    SoundManager::GetInstance().PlayMusic("dungeonAir");
    SoundManager::GetInstance().PlayMusic("jungleAmbience");
    SetMusicVolume(SoundManager::GetInstance().GetMusic("jungleAmbience"), 0.5f);


    controlPlayer = true; //start as player //hit tab for free camera

    // Camera //we pass camera around like crazy
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
            if (IsKeyPressed(KEY_ESCAPE)) currentGameState = GameState::Playing;
            if (IsKeyPressed(KEY_UP)) selectedOption = (selectedOption - 1 + 3) % 3;
            if (IsKeyPressed(KEY_DOWN)) selectedOption = (selectedOption + 1) % 3;

            if (IsKeyPressed(KEY_ENTER)) {
                if (selectedOption == 0) {
                    InitLevel(levels[levelIndex], camera);
                    currentGameState = GameState::Playing;
                } else if (selectedOption == 1) {
                    levelIndex = (levelIndex + 1) % levels.size(); // Cycle through levels
                } else if (selectedOption == 2) {
                    currentGameState = GameState::Quit;
                }
            }


            BeginDrawing();
            DrawMenu(backDrop, selectedOption, levelIndex);
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
        UpdateShaders(camera);
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
            HandleDungeon(deltaTime);
            
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
        BeginTextureMode(sceneTexture);
        ClearBackground(SKYBLUE);
        float farClip = isDungeon ? 10000.0f : 50000.0f;//10k in dungeons 50k outside

        BeginCustom3D(camera, farClip);
        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(skyModel, camera.position, 10000.0f, WHITE); //draw skybox with no depthmask or test or backface culling, leave backfaceculling off. 
        rlEnableDepthMask(); rlEnableDepthTest();
        rlSetBlendMode(BLEND_ALPHA); //required 
        //rlEnableColorBlend(); //not sure

        DrawDungeonFloor();
        DrawDungeonWalls();
        DrawDungeonCeiling(floorTile);
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
            DrawModel(waterModel, waterPos, 1.0f, WHITE); 
            DrawModel(bottomPlane, bottomPos, 1.0f, DARKBLUE); //a second plane below water plane. to prevent seeing through the world when looking down.
            DrawBoat(player_boat);
            DrawTrees(trees, palmTree, palm2, shadowQuad); 
            DrawBushes(bushes, shadowQuad);
        }


        EndBlendMode();
        EndMode3D(); //////////////////EndMode3

        rlDisableDepthTest();
        EndTextureMode();//////end drawing to texture

        BeginTextureMode(depthEffectTexture);
            BeginShaderMode(depthShader);
                SetShaderValueTexture(depthShader, sceneTextureLoc, sceneTexture.texture);
                SetShaderValueTexture(depthShader, sceneDepthLoc, sceneTexture.depth);

                DrawTextureRec(sceneTexture.texture, 
                    (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                    (Vector2){0, 0}, WHITE);
            EndShaderMode();
        EndTextureMode();

                // === POSTPROCESS TO postProcessTexture ===
        BeginTextureMode(postProcessTexture);
            BeginShaderMode(fogShader); // original post shader
                DrawTextureRec(depthEffectTexture.texture, 
                    (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                    (Vector2){0, 0}, WHITE);
            EndShaderMode();
        EndTextureMode(); // postProcessTexture now contains processed scene

        // === Final draw to screen ===
        BeginDrawing();
            ClearBackground(WHITE);

            BeginShaderMode(bloomShader); // second pass
                DrawTextureRec(postProcessTexture.texture,
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
            if (player.activeWeapon == WeaponType::MagicStaff) DrawMagicIcon(player);
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
    UnloadAllResources();
    CloseAudioDevice();
    CloseWindow();

    //system("pause"); // ← waits for keypress
    return 0;
}
