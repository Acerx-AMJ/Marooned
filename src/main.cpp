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






void BeginCustom3D(Camera3D camera, float farClip) {
    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    float nearClip = 60.0f; //20 wider than the capsule. to 50k outside, 10k in dungeons
    Matrix proj = MatrixPerspective(DEG2RAD * camera.fovy, (float)GetScreenWidth() / (float)GetScreenHeight(), nearClip, farClip);

    rlMultMatrixf(MatrixToFloat(proj));

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(view));
}

void ClearLevel() {
    isDungeon = false;
    ClearDungeon();
  
    dungeonEntrances.clear();
    RemoveAllVegetation();
    removeAllCharacters();
    if (terrainMesh.vertexCount > 0) UnloadMesh(terrainMesh); //unload mesh when switching levels. 
    if (heightmap.data != nullptr) UnloadImage(heightmap);
 
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
        d.doorType = DoorType::GoToNext; 

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

        doorways.push_back(dw);
    }

}


void InitLevel(const LevelData& level, Camera camera) {
    //Called when starting game and changing level. init the level you pass. 
    ClearLevel();//clears everything. 
    camera.position = player.position;
    levelIndex = level.levelIndex; //update current level index to new level. 

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
        swordModel.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = swordClean; //start with a clean sword
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
        GenerateLightSources(floorHeight);
        GenerateDoorways(floorHeight, levelIndex); //calls generate doors from archways
        GenerateSkeletonsFromImage(dungeonEnemyHeight); //165
        GeneratePiratesFromImage(dungeonEnemyHeight-100);
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
  
    InitPlayer(player, resolvedSpawn); //start at green pixel if there is one. otherwise level.startPos or first startPos
}

void SpawnRaptor(Vector3 pos) {
    Character raptor(pos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);
    enemies.push_back(raptor);
    enemyPtrs.push_back(&enemies.back());
}


Color ColorLerp(Color a, Color b, float t) {
    
    Color result;
    result.r = (unsigned char)Lerp((float)a.r, (float)b.r, t);
    result.g = (unsigned char)Lerp((float)a.g, (float)b.g, t);
    result.b = (unsigned char)Lerp((float)a.b, (float)b.b, t);
    result.a = (unsigned char)Lerp((float)a.a, (float)b.a, t);
    return result;
}



void UpdateEnemies(float deltaTime) {
    for (Character& e : enemies){
        e.Update(deltaTime, player, heightmap, terrainScale);
    }
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
        if (b.IsAlive()) b.Draw();
    }

}


void DrawHealthBar(){

    float healthPercent = (float)player.currentHealth / player.maxHealth;
    healthPercent = Clamp(healthPercent, 0.0f, 1.0f); // safety first
    int barWidth = 300;
    int barHeight = 30;
    int barX = GetScreenWidth()/3 - barWidth/2;
    int barY = GetScreenHeight() -80;

    Rectangle healthBarFull = { (float)barX, (float)barY, (float)barWidth, (float)barHeight };

    Rectangle healthBarCurrent = { 
        (float)barX, 
        (float)barY, 
        (float)(barWidth * healthPercent), 
        (float)barHeight 
    };

    // Background frame 
    DrawRectangleLines(barX - 1, barY - 1, barWidth + 2, barHeight + 2, WHITE);

    // Tint white to red based on health
    //Color barColor = GetHealthBarColor(healthPercent);

    Color barColor = WHITE;
    if (healthPercent < 0.25f) {
        float pulse = sin(GetTime() * 10.0f) * 0.5f + 0.5f; // 0..1
        barColor = ColorLerp(WHITE, RED, pulse);
    }

    // Current health fill
    DrawRectangleRec(healthBarCurrent, barColor);

}

void DrawStaminaBar(){
    float staminaPercent = player.stamina / player.maxStamina;
    staminaPercent = Clamp(staminaPercent, 0.0f, 1.0f);

    int staminaBarWidth = 300;
    int staminaBarHeight = 10;
    int staminaBarX = GetScreenWidth()/3 - staminaBarWidth/2;
    int staminaBarY = GetScreenHeight() - 40;  

    Rectangle staminaBarBack = { (float)staminaBarX, (float)staminaBarY, (float)staminaBarWidth, (float)staminaBarHeight };
    Rectangle staminaBarCurrent = {
        (float)staminaBarX,
        (float)staminaBarY,
        (float)(staminaBarWidth * staminaPercent),
        (float)staminaBarHeight

    
    };

    // Outline
    DrawRectangleLines(staminaBarX - 1, staminaBarY - 1, staminaBarWidth + 2, staminaBarHeight + 2, DARKGRAY);

    // Color based on stamina
    Color barColor = ColorLerp((Color){50, 50, 150, 255}, BLUE, staminaPercent);  // subtle fade

    DrawRectangleRec(staminaBarCurrent, barColor);

}





void DrawMenu() {
    ClearBackground(BLACK);
    
    DrawTexturePro(
        backDrop,
        Rectangle{ 0, 0, (float)backDrop.width, (float)backDrop.height },
        Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
        Vector2{ 0, 0 },
        0.0f,
        WHITE
    );

    //float middle = GetScreenWidth()/2 - 150;
    const char* title = "MAROONED";
    int fontSize = 60;
    int titleX = GetScreenWidth() / 2 - MeasureText(title, fontSize) / 2;
    DrawText(title, titleX, 180, fontSize, GREEN);

    DrawText(selectedOption == 0 ? "> Start" : "  Start", titleX, 280, 30, WHITE);
    
    DrawText(
        TextFormat("%s Level: %s", selectedOption == 1 ? ">" : " ", levels[levelIndex].name.c_str()),
        titleX, 330, 30, YELLOW
    );

    DrawText(selectedOption == 2 ? "> Quit" : "  Quit", titleX, 380, 30, WHITE);
}



void UpdateCollectables(Camera& camera, float deltaTime) { //update and DRAW
    for (int i = 0; i < collectables.size(); i++) {
        collectables[i].Update(deltaTime);

        // Draw correct icon
        if (collectables[i].type == CollectableType::HealthPotion) {
            collectables[i].Draw(healthPotTexture, camera, 40.0f);
        }
        else if (collectables[i].type == CollectableType::Key) {
            collectables[i].Draw(keyTexture, camera, 80.0f);//double the scale for keys
        }
        else if (collectables[i].type == CollectableType::Gold) {
            collectables[i].Draw(coinTexture, camera, 40);
        }

        // Pickup logic
        if (collectables[i].CheckPickup(player.position, 180.0f)) {
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
        LightSource bulletLight;
        bulletLight.position = bullet.GetPosition();
        bulletLight.range = 300.0f;
        bulletLight.intensity = 1.2f;
        bulletLight.lifeTime = 0.5f;
        bulletLight.age = 0.0f;
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

void DrawBillboards(Camera3D camera) {
    // 1️⃣ Sort all transparent billboards back-to-front
    std::sort(billboardRequests.begin(), billboardRequests.end(),
        [&camera](const BillboardDrawRequest& a, const BillboardDrawRequest& b) {
            return a.distanceToCamera > b.distanceToCamera;
        });

    for (const BillboardDrawRequest& req : billboardRequests) {
        rlDisableDepthMask();

        switch (req.type) {
            case Billboard_FacingCamera:
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
            case Billboard_FixedFlat:
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



    // 3️⃣ Clear list for next frame
    billboardRequests.clear();
}




// void DrawAllEnemies(Camera& camera){
//     //Sort all enemies in enemyPtrs before drawing. 
//     Vector3 camPos = camera.position;
//     std::sort(enemyPtrs.begin(), enemyPtrs.end(),
//         [camPos](Character* a, Character* b) {
//             float distA = Vector3DistanceSqr(a->position, camPos);
//             float distB = Vector3DistanceSqr(b->position, camPos);
//             return distA > distB; // draw furthest first
//         });

//     for (Character* enemy : enemyPtrs) {        
//         enemy->Draw(camera);
        
//     }
// }


void DrawTimer(){
    int minutes = (int)(ElapsedTime / 60.0f);
    int seconds = (int)ElapsedTime % 60;

    char buffer[16];
    sprintf(buffer, "Time: %02d:%02d", minutes, seconds);

    DrawText(buffer, GetScreenWidth()-150, 30, 20, WHITE); 
}






int main() { 
    InitWindow(1600, 900, "Marooned");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadAllResources();
    
    SetExitKey(KEY_NULL); //Escape brings up menu, not quit
    Music dungeonAir = LoadMusicStream("assets/sounds/dungeonAir.ogg");
    Music jungleAmbience = LoadMusicStream("assets/sounds/jungleSounds.ogg"); 
    PlayMusicStream(jungleAmbience); // Starts playback
    PlayMusicStream(dungeonAir);
    SetMusicVolume(jungleAmbience, 0.5f);
    SetMusicVolume(dungeonAir, 1.0f);
    controlPlayer = true; //start as player

    // Camera //we pass camera around like crazy maybe make a global camera. 
    Camera3D camera = { 0 };
    camera.position = startPosition;
    camera.target = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    DisableCursor();



    //SetupFogShader(camera);
    //InitChests();
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
            DrawMenu();
            EndDrawing();

            if (currentGameState == GameState::Quit) break;
            

            continue; // skip the rest of the game loop
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentGameState = GameState::Menu;
        }
        float deltaTime = GetFrameTime();
        UpdateInputMode(); //handle both gamepad and keyboard/mouse
        debugControls(); //press P to remove all vegetation, Press O to regenerate raptors, Press L to print player position
        UpdateShaders(camera);
        sortTrees(camera); //sort trees by distance to player, draw closest trees last.
        
        UpdateFade(deltaTime, camera); //triggers init level
        UpdateBullets(camera, deltaTime);
        UpdateDecals(deltaTime);
        UpdateBoat(player_boat, deltaTime);
        UpdateEnemies(deltaTime);
        UpdateDungeonChests();

        // UpdateRaptors(deltaTime);
        // UpdateSkeletons(deltaTime);
        // UpdatePirates(deltaTime);

        CheckBulletHits(camera); //bullet collision
        TreeCollision(camera); //player and raptor vs tree
        DoorCollision();
        SpiderWebCollision();
        barrelCollision();
        ChestCollision();
        
        pillarCollision();
        HandleMeleeHitboxCollision(camera);
        HandleDoorInteraction(camera);
        //billboardRequests.clear();
        // GatherEnemies(camera);
        // GatherDungeonFires(camera, deltaTime);
        // GatherWebs(camera);
        // GatherDecals(camera, decals);

        GatherTransparentDrawRequests(camera, player.weapon, deltaTime);
        DrawTransparentDrawRequests(camera);

        if (!isDungeon) UpdateMusicStream(jungleAmbience);
        if (isDungeon){
            HandleDungeon(deltaTime);
            UpdateMusicStream(dungeonAir);
            
        }

        if (IsGamepadAvailable(0)) {  //free camera with gamepad
            UpdateCameraWithGamepad(camera);
        }



        HandleCameraPlayerToggle(camera, player, controlPlayer);
        UpdateCameraAndPlayer(camera, player, controlPlayer, deltaTime);
        // During render loop:

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
        rlEnableColorBlend(); //not sure

        DrawDungeonFloor();
        DrawDungeonWalls(wall);
        DrawDungeonCeiling(floorTile);
        DrawDungeonBarrels();
        //DrawSpiderWebs(camera);
        DrawDungeonChests();
        
        DrawDungeonDoorways(doorWay); 
        DrawPlayer(player, camera);

        //draw things with transparecy last.
        rlDisableDepthMask();
        //DrawAllEnemies(camera);
        DrawBillboards(camera);
        DrawBullets(camera); //and decals //draw bullets and decals after enemies,
        UpdateCollectables(camera, deltaTime); //Update and draw
        DrawDungeonPillars(deltaTime, camera); //light sources become invisible when behind enemies, but it's better then enemies being invisible being behind light sources. 
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
            ClearBackground(BLACK);

            BeginShaderMode(bloomShader); // second pass
                DrawTextureRec(postProcessTexture.texture,
                    (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                    (Vector2){0, 0}, WHITE);
            EndShaderMode();

        ///2D on top of render texture
        if (pendingLevelIndex != -1) { //fading out...draw loading text and nothing else. 
            DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20) / 2, GetScreenHeight() / 2, 20, WHITE);
        }else{
            DrawHealthBar();
            DrawStaminaBar();
            DrawText(TextFormat("Gold: %d", (int)player.displayedGold), 32, GetScreenHeight()-120, 30, GOLD); 
            if (debugInfo){//press ~ to hide debug info
                DrawTimer(); 
                DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
                DrawText(currentInputMode == InputMode::Gamepad ? "Gamepad" : "Keyboard", 10, 30, 20, LIGHTGRAY);
                DrawText("PRESS TAB FOR FREE CAMERA", GetScreenWidth()/2 + 280, 30, 20, WHITE);

            }

            player.inventory.DrawInventoryUIWithIcons(itemTextures, slotOrder, 20, GetScreenHeight() - 80, 64);

            player.inventory.DrawInventoryUI();
            
        }

        EndDrawing();

    }

    // Cleanup
    ClearLevel();
    UnloadAllResources();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
