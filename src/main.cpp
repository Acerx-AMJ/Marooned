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



void BeginCustom3D(Camera3D camera, float farClip) {
    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    float nearClip = 60.0f; //20 wider than the capsule. to 50k 
    Matrix proj = MatrixPerspective(DEG2RAD * camera.fovy, (float)GetScreenWidth() / (float)GetScreenHeight(), nearClip, farClip);

    rlMultMatrixf(MatrixToFloat(proj));

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(view));
}

void UpdateCustomCamera(Camera3D* camera, float deltaTime) {
    //Free camera control
    static float yaw = 0.0f;
    static float pitch = 0.0f;

    float moveSpeed = 1000.0f * deltaTime;
    float lookSensitivityMouse = 0.03f;
    float lookSensitivityGamepad = 100.0f * deltaTime;

    // Get input from both controller and mouse
    Vector2 mouseDelta = GetMouseDelta();
    yaw   += mouseDelta.x * lookSensitivityMouse;
    pitch += -mouseDelta.y * lookSensitivityMouse;


    if (IsGamepadAvailable(0)) {
        float rightX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        float rightY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        yaw   += rightX * lookSensitivityGamepad;
        pitch += -rightY * lookSensitivityGamepad;
    }

    // Clamp pitch to avoid flipping
    pitch = Clamp(pitch, -89.0f, 89.0f);

    // Direction vector based on yaw/pitch
    Vector3 forward = {
        cosf(pitch * DEG2RAD) * cosf(yaw * DEG2RAD),
        sinf(pitch * DEG2RAD),
        cosf(pitch * DEG2RAD) * sinf(yaw * DEG2RAD)
    };
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera->up));

    // Combined movement input (keyboard + gamepad left stick)
    float inputX = 0.0f;
    float inputZ = 0.0f;
    float inputY = 0.0f;

    if (IsKeyDown(KEY_W)) inputZ += 1.0f;
    if (IsKeyDown(KEY_S)) inputZ -= 1.0f;
    if (IsKeyDown(KEY_A)) inputX -= 1.0f;
    if (IsKeyDown(KEY_D)) inputX += 1.0f;
    if (IsKeyDown(KEY_SPACE)) inputY += 1.0f; //fly up and down
    if (IsKeyDown(KEY_LEFT_SHIFT)) inputY -= 1.0f;

    if (IsGamepadAvailable(0)) {
        inputX += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        inputZ += -GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y); // Y axis is inverted
    }

    // Apply movement
    Vector3 movement = Vector3Zero();
    movement = Vector3Add(movement, Vector3Scale(right, inputX * moveSpeed));
    movement = Vector3Add(movement, Vector3Scale(forward, inputZ * moveSpeed));
    movement.y += inputY * moveSpeed;

    camera->position = Vector3Add(camera->position, movement);
    camera->target = Vector3Add(camera->position, forward);
}

void UpdateCameraAndPlayer(Camera& camera, Player& player, bool controlPlayer, float deltaTime) {
    if (controlPlayer) {
        UpdatePlayer(player, GetFrameTime(), terrainMesh, camera);
        DisableCursor();

        float yawRad = DEG2RAD * player.rotation.y;
        float pitchRad = DEG2RAD * player.rotation.x;

        Vector3 forward = {
            cosf(pitchRad) * sinf(yawRad),
            sinf(pitchRad),
            cosf(pitchRad) * cosf(yawRad)
        };

        float camYOffset = player.isSwimming ? -40.0f : 1.5f;  // swimming lowers the camera
        camera.position = Vector3Add(player.position, (Vector3){ 0, camYOffset, 0 });
        //camera.position = Vector3Add(player.position, (Vector3){ 0, 1.5f, 0 });
        camera.target = Vector3Add(camera.position, forward);


    } else {
        UpdateCustomCamera(&camera, deltaTime);
        if (player.onBoard) {
            player.position = Vector3Add(player_boat.position, {0, 200.0f, 0});
        }
    }
}


void HandleCameraPlayerToggle(Camera& camera, Player& player, bool& controlPlayer) {
    static Vector3 savedForward;

    if (IsKeyPressed(KEY_TAB)) {
        controlPlayer = !controlPlayer;

        if (controlPlayer) {
            savedForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            float yaw = atan2f(savedForward.x, savedForward.z) * RAD2DEG;
            float pitch = -asinf(savedForward.y) * RAD2DEG;

            player.position = camera.position;
            player.velocity = { 0 };
            player.rotation.y = yaw;
            player.rotation.x = Clamp(pitch, -89.0f, 89.0f);
        } else {
            camera.position = Vector3Add(player.position, { 0, 1.5f, 0 });
            camera.target = Vector3Add(camera.position, savedForward);
        }
    }

}





void SpawnRaptor(Vector3 pos) {
    Character raptor(pos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);
    raptors.push_back(raptor);
}


Color ColorLerp(Color a, Color b, float t) {
    
    Color result;
    result.r = (unsigned char)Lerp((float)a.r, (float)b.r, t);
    result.g = (unsigned char)Lerp((float)a.g, (float)b.g, t);
    result.b = (unsigned char)Lerp((float)a.b, (float)b.b, t);
    result.a = (unsigned char)Lerp((float)a.a, (float)b.a, t);
    return result;
}






void UpdateRaptors(float deltaTime){
    for (Character& raptor : raptors) {
        raptor.Update(deltaTime, player,  heightmap, terrainScale);
    }
}

void UpdateSkeletons(float deltaTime){
    for (Character& skeleton : skeletons) {
        skeleton.Update(deltaTime, player, heightmap, terrainScale);
    }
}

void UpdatePirates(float deltaTime){
    for (Character& pirate : pirates){
        pirate.Update(deltaTime, player, heightmap, terrainScale);
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

    for (auto& d : decals) {
        
        d.Draw(camera);

    }

}

void PopulateEnemyPtrs(){
    enemyPtrs.clear();

    for (Character& s : skeletons) enemyPtrs.push_back(&s);
    for (Character& p : pirates) enemyPtrs.push_back(&p);
    for (Character& r : raptors) enemyPtrs.push_back(&r);
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

void ClearLevel() {
    isDungeon = false;
    ClearDungeon();
    dungeonEntrances.clear();
    RemoveAllVegetation();
    removeAllCharacters();
    if (terrainMesh.vertexCount > 0) UnloadMesh(terrainMesh); //unload mesh when switching levels. 
    if (heightmap.data != nullptr) UnloadImage(heightmap);

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

void UpdateCollectables(Camera& camera, float deltaTime) {
    for (int i = 0; i < collectables.size(); i++) {
        collectables[i].Update(deltaTime);

        // Draw correct icon
        if (collectables[i].type == CollectableType::HealthPotion) {
            collectables[i].Draw(healthPotTexture, camera, 40.0f);
        }
        else if (collectables[i].type == CollectableType::Key) {
            collectables[i].Draw(keyTexture, camera, 80.0f);//double the scale for keys
        }

        // Pickup logic
        if (collectables[i].CheckPickup(player.position, 100.0f)) {
            if (collectables[i].type == CollectableType::HealthPotion) {
                player.inventory.AddItem("HealthPotion");
                SoundManager::GetInstance().Play("clink");
            }
            else if (collectables[i].type == CollectableType::Key) {
                player.inventory.AddItem("GoldKey");
                SoundManager::GetInstance().Play("key");
            }

            collectables.erase(collectables.begin() + i);
            i--;
        }
    }
}



void InitLevel(const LevelData& level, Camera camera) {
    //Called when starting game and changing level. init the level you pass. 
    ClearLevel();//clears everything. 
    camera.position = player.position;
    levelIndex = level.levelIndex; //update current level index to new level. 

    if (!isDungeon){ // Generate the terrain mesh and model from the heightmap
       
        // Load and format the heightmap image
        heightmap = LoadImage(level.heightmapPath.c_str());
        ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
        terrainScale = level.terrainScale;
        terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
        terrainModel = LoadModelFromMesh(terrainMesh);
        terrainModel.materials[0].shader = terrainShader;
        dungeonEntrances = level.entrances; //get level entrances from level data

        generateRaptors(level.raptorCount, level.raptorSpawnCenter, 6000);
        PopulateEnemyPtrs();//add raptors to all enemies list. 
        GenerateEntrances();
        generateVegetation(); 
        InitBoat(player_boat, boatPosition);
    }

   
    if (level.isDungeon){
        isDungeon = true;

        LoadDungeonLayout(level.dungeonPath);
        ConvertImageToWalkableGrid(dungeonImg);
        GenerateFloorTiles(floorHeight);//80
        GenerateWallTiles(wallHeight); //model is 400 tall with origin at it's center, so wallHeight is floorHeight + model height/2. 270
        GenerateCeilingTiles(ceilingHeight);//400
        GenerateBarrels(floorHeight); 
        GeneratePotions(floorHeight);
        GenerateKeys(floorHeight);
        GenerateLightSources(floorHeight);
        GenerateDoorways(floorHeight, levelIndex); //calls generate doors from archways
        GenerateSkeletonsFromImage(dungeonEnemyHeight); //165
        GeneratePiratesFromImage(dungeonEnemyHeight);
        PopulateEnemyPtrs(); //add pirates and skeletons to the enemyPtrs vector. iterate this list for collision, sorting, drawing, but not removal.
      
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

    //lightBullets(deltaTime);


     //Vertex Color Lighting
    // === Update tints with updated light list ===
   
    UpdateWallTints(player.position);
    UpdateCeilingTints(player.position);
    UpdateFloorTints(player.position);
    UpdateBarrelTints(player.position);
    UpdateDoorwayTints(player.position);
    UpdateDoorTints(player.position);
}



void DrawAllEnemies(Camera& camera){
    //Sort all enemies in enemyPtrs before drawing. 
    Vector3 camPos = camera.position;
    std::sort(enemyPtrs.begin(), enemyPtrs.end(),
        [camPos](Character* a, Character* b) {
            float distA = Vector3DistanceSqr(a->position, camPos);
            float distB = Vector3DistanceSqr(b->position, camPos);
            return distA > distB; // draw furthest first
        });

    for (Character* enemy : enemyPtrs) {        
        enemy->Draw(camera);
        
    }


}





int main() {
    //SetConfigFlags(FLAG_MSAA_4X_HINT); //anti aliasing, I see no difference. 
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

    Vector3 terrainPosition = { //center the terrain around 0, 0, 0
            -terrainScale.x / 2.0f,
            0.0f,
            -terrainScale.z / 2.0f
    }; 

    
    //main game loop
    while (!WindowShouldClose()) {
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
        
        UpdateRaptors(deltaTime);
        UpdateSkeletons(deltaTime);
        UpdatePirates(deltaTime);

        CheckBulletHits(camera); //bullet collision
        TreeCollision(camera); //player and raptor vs tree
        DoorCollision();
        barrelCollision();
        pillarCollision();
        HandleMeleeHitboxCollision(camera);
        HandleDoorInteraction(camera);

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

        BeginCustom3D(camera, 50000.0f);
        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(skyModel, camera.position, 10000.0f, WHITE); //draw skybox with no depthmask or test or backface culling, leave backfaceculling off. 
        rlEnableDepthMask(); rlEnableDepthTest();
        rlSetBlendMode(BLEND_ALPHA);
        rlEnableColorBlend();

        DrawDungeonFloor(floorTile);
        DrawDungeonWalls(wall);
        DrawDungeonBarrels();

        DrawDungeonDoorways(doorWay); 
        DrawDungeonCeiling(floorTile);

        //draw things with transparecy last.
        UpdateCollectables(camera, deltaTime); //Update and draw
        DrawDungeonPillars(deltaTime, camera);
       
        if (!isDungeon) {
            DrawModel(terrainModel, terrainPosition, 1.0f, WHITE);
            DrawModel(waterModel, waterPos, 1.0f, WHITE); 
            DrawModel(bottomPlane, bottomPos, 1.0f, DARKBLUE); //a second plane below water plane. to prevent seeing through the world when looking down.
            DrawBoat(player_boat);
            DrawTrees(trees, palmTree, palm2, shadowQuad); 
            DrawBushes(bushes, shadowQuad);
        }
        

        // drawRaptors(camera); //sort and draw raptors
        // drawSkeletons(camera);
        // drawPirates(camera);

        DrawAllEnemies(camera);
        DrawPlayer(player, camera);
        

        DrawBullets(camera); //and decals

        EndBlendMode();
        EndMode3D(); //////////////////EndMode3

        rlDisableDepthTest();

        EndTextureMode();//////end drawing to texture

        // === POSTPROCESS TO SCREEN ===
        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(fogShader);
        DrawTextureRec(sceneTexture.texture, 
            (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
            (Vector2){0, 0}, WHITE);
        EndShaderMode();


        ///2D on top of render texture
        if (pendingLevelIndex != -1) { //fading out...draw loading text and nothing else. 
            DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20) / 2, GetScreenHeight() / 2, 20, WHITE);
        }else{
            DrawHealthBar();
            DrawStaminaBar();
            DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
            DrawText(currentInputMode == InputMode::Gamepad ? "Gamepad" : "Keyboard", 10, 30, 20, LIGHTGRAY);
            DrawText("PRESS TAB FOR FREE CAMERA", GetScreenWidth()/2 + 250, 30, 20, WHITE);
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
