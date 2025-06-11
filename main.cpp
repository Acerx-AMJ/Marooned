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

#define GLSL_VERSION 330



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



    if (IsKeyPressed(KEY_TAB)) {
        controlPlayer = !controlPlayer;

        if (controlPlayer) {
            // Entering player mode — copy camera orientation to player
            Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            float yaw = atan2f(camForward.x, camForward.z) * RAD2DEG;
            float pitch = -asinf(camForward.y) * RAD2DEG;

            player.position = camera.position;
            player.velocity = { 0 };
            player.rotation.y = yaw;
            player.rotation.x = Clamp(pitch, -89.0f, 89.0f);
        } else {
            // Exiting player mode — copy player orientation back to camera
            float yawRad = DEG2RAD * player.rotation.y;
            float pitchRad = DEG2RAD * player.rotation.x;

            Vector3 forward = {
                cosf(pitchRad) * sinf(yawRad),
                sinf(pitchRad),
                cosf(pitchRad) * cosf(yawRad)
            };

            camera.position = Vector3Add(player.position, (Vector3){ 0, 1.5f, 0 });
            camera.target = Vector3Add(camera.position, forward);
        }
    }
}



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

void SpawnRaptor(Vector3 pos) {
    Character raptor(pos, &raptorTexture, 200, 200, 1, 0.5f, 0.5f);
    raptors.push_back(raptor);
}



void drawRaptors(Camera& camera){

    std::sort(raptorPtrs.begin(), raptorPtrs.end(), [&](Character* a, Character* b) {
        float distA = Vector3Distance(camera.position, a->position);
        float distB = Vector3Distance(camera.position, b->position);
        return distA > distB; // Farthest first
    });

    for (Character* raptor : raptorPtrs){
        raptor->Draw(camera);
    }



}

Color ColorLerp(Color a, Color b, float t) {
    Color result;
    result.r = (unsigned char)Lerp((float)a.r, (float)b.r, t);
    result.g = (unsigned char)Lerp((float)a.g, (float)b.g, t);
    result.b = (unsigned char)Lerp((float)a.b, (float)b.b, t);
    result.a = (unsigned char)Lerp((float)a.a, (float)b.a, t);
    return result;
}



bool CheckCollisionPointBox(Vector3 point, BoundingBox box) {
    return (
        point.x >= box.min.x && point.x <= box.max.x &&
        point.y >= box.min.y && point.y <= box.max.y &&
        point.z >= box.min.z && point.z <= box.max.z
    );
}



void UpdateRaptors(float deltaTime){
    for (Character& raptor : raptors) {
        raptor.Update(deltaTime, player.position, player,  heightmap, terrainScale, raptorPtrs);
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

void CheckBulletHits(Camera& camera) {
    for (Bullet& b : activeBullets) {
        if (!b.IsAlive()) continue;

        for (Character* r : raptorPtrs) {
            if (r->isDead) continue;

            BoundingBox box = r->GetBoundingBox();
            if (CheckCollisionPointBox(b.GetPosition(), box)) {
                r->TakeDamage(25);
                b.kill(camera);
                //blood decals on raptor hit
                Vector3 camDir = Vector3Normalize(Vector3Subtract(b.GetPosition(), camera.position));
                Vector3 offsetPos = Vector3Add(b.GetPosition(), Vector3Scale(camDir, -150.0f));
  

                decals.emplace_back(offsetPos, DecalType::Smoke, &bloodSheet, 7, 1.2f, 0.2f, 60.0f);
                break;
            }
        }
    }
    //bullet collision with dungeon walls. 
    for (WallRun& w : wallRunColliders){
        for (Bullet& b: activeBullets){
            if (CheckCollisionPointBox(b.GetPosition(), w.bounds)){
                b.kill(camera);
            }
        }
    }
}

void DrawBullets() {
    for (const Bullet& b : activeBullets) {
        if (b.IsAlive()) b.Draw();
    }
}

bool CheckBulletHitsTree(const TreeInstance& tree, const Vector3& bulletPos) {
    Vector3 treeBase = {
        tree.position.x + tree.xOffset,
        tree.position.y + tree.yOffset,
        tree.position.z + tree.zOffset
    };

    // Check vertical overlap
    if (bulletPos.y < treeBase.y || bulletPos.y > treeBase.y + tree.colliderHeight) {
        return false;
    }

    // Check horizontal distance from tree trunk center
    float dx = bulletPos.x - treeBase.x;
    float dz = bulletPos.z - treeBase.z;
    float horizontalDistSq = dx * dx + dz * dz;

    return horizontalDistSq <= tree.colliderRadius * tree.colliderRadius;
}



void TreeCollision(Camera& camera){


    for (TreeInstance& tree : trees) {
        if (Vector3DistanceSqr(tree.position, player.position) < 500 * 500) {
            if (CheckTreeCollision(tree, player.position)) {
                ResolveTreeCollision(tree, player.position);
            }
        }
    }

    for (Character* raptor : raptorPtrs){
        for (TreeInstance& tree : trees) {
            if (Vector3DistanceSqr(tree.position, raptor->position) < 500*500) {
                if (CheckTreeCollision(tree, raptor->position)) {
                    ResolveTreeCollision(tree, raptor->position);
                    
                }
            }
        }

    }



    for (TreeInstance& tree : trees) {
        for (Bullet& bullet : activeBullets){
            if (!bullet.IsAlive()) continue; // <-- early out for dead bullets
            if (Vector3DistanceSqr(tree.position, bullet.GetPosition()) < 500 * 500) {
                if (CheckBulletHitsTree(tree, bullet.GetPosition())) {
                   
                   
                    //Tree hit by bullet. Play a sound. 
                    bullet.kill(camera);
                    break;
                }

            }
 
        }
    }

}



void DrawHealthBar(){

    float healthPercent = (float)player.currentHealth / player.maxHealth;
    healthPercent = Clamp(healthPercent, 0.0f, 1.0f); // safety first
    int barWidth = 300;
    int barHeight = 30;
    int barX = GetScreenWidth()/2 - barWidth/2;
    int barY = 20;

    Rectangle healthBarFull = { (float)barX, (float)barY, (float)barWidth, (float)barHeight };

    Rectangle healthBarCurrent = { 
        (float)barX, 
        (float)barY, 
        (float)(barWidth * healthPercent), 
        (float)barHeight 
    };

    // Background frame (gray or black)
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
    int staminaBarX = GetScreenWidth()/2 - staminaBarWidth/2;
    int staminaBarY = 60;  // health bar was probably at y = 20

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
    RemoveAllVegetation();
    removeAllRaptors();
    UnloadImage(heightmap);
    UnloadModel(terrainModel);
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




void InitLevel(const LevelData& level, Camera camera) {
    ClearLevel();
    // Load and format the heightmap image
    heightmap = LoadImage(level.heightmapPath.c_str());
    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

    // Generate the terrain mesh and model from the heightmap
    terrainScale = level.terrainScale;
    terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
    terrainModel = LoadModelFromMesh(terrainMesh);

    // Generate vegetation like trees and bushes
    generateVegetation();

    // Initialize the player at the specified start position
    
    camera.position = player.position;
    // Generate raptors around the given spawn center
   

    InitBoat(player_boat, boatPosition);
    
    terrainModel.materials[0].shader = terrainShader;


    if (level.isDungeon){
        isDungeon = true;
        LoadDungeonLayout("assets/maps/map4.png");
        
        GenerateFloorTiles(200.0f, floorHeight);
        GenerateWallTiles(200.0f, floorHeight);
        GenerateCeilingTiles(400.0f);
        GenerateBarrels(200, floorHeight);
        GenerateLightSources(200, floorHeight);
        generateRaptors(level.raptorCount,level.raptorSpawnCenter, 8000);

    }else{
        generateRaptors(level.raptorCount, level.raptorSpawnCenter, 3000);
    }
    //player.position = FindSpawnPoint(dungeonPixels, dungeonWidth, dungeonHeight, 200, floorHeight);
    Vector3 resolvedSpawn = level.startPosition; // default fallback

    Vector3 pixelSpawn = FindSpawnPoint(dungeonPixels, dungeonWidth, dungeonHeight, 200, floorHeight);
    if (pixelSpawn.x != 0 || pixelSpawn.z != 0) {
        resolvedSpawn = pixelSpawn;
        //Overriding start position with green pixel
    }
    InitPlayer(player, resolvedSpawn);

}

void WallCollision(){
    for (const WallRun& run : wallRunColliders) {
        if (CheckCollisionBoxSphere(run.bounds, player.position, player.radius)) {
            ResolveBoxSphereCollision(run.bounds, player.position, player.radius);
        }

        for (Character& r : raptors){
            if (CheckCollisionBoxSphere(run.bounds, r.position, 100)){
                ResolveBoxSphereCollision(run.bounds, r.position, 100);
            }
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

void HandleDungeon(float deltaTime) {
    WallCollision();

    //lightBullets(deltaTime);
    
    // === Update tints with updated light list ===
    UpdateWallTints(player.position);
    UpdateCeilingTints(player.position);
    UpdateFloorTints(player.position);
    UpdateBarrelTints(player.position);
}



int main() {
    //SetConfigFlags(FLAG_MSAA_4X_HINT); //anti aliasing, I see no difference. 
    InitWindow(1600, 800, "Marooned");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadAllResources();
    SetExitKey(KEY_NULL);
    Music dungeonAir = LoadMusicStream("assets/sounds/dungeonAir.ogg");
    Music jungleAmbience = LoadMusicStream("assets/sounds/jungleSounds.ogg"); 
    PlayMusicStream(jungleAmbience); // Starts playback
    PlayMusicStream(dungeonAir);
    SetMusicVolume(jungleAmbience, 0.5f);
    SetMusicVolume(dungeonAir, 1.0f);
    controlPlayer = true; //start as player

    
    // Camera
    Camera3D camera = { 0 };
    camera.position = startPosition;
    camera.target = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    DisableCursor();
    
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
        if (!isDungeon) UpdateMusicStream(jungleAmbience);
        
        UpdateBullets(camera, deltaTime);
        UpdateDecals(deltaTime);
        CheckBulletHits(camera);
        UpdateDecals(deltaTime);
        UpdateBoat(player_boat, deltaTime);
        UpdateRaptors(deltaTime);
        TreeCollision(camera); //player and raptor vs tree
        
        if (isDungeon){
            HandleDungeon(deltaTime);
            UpdateMusicStream(dungeonAir);
            

        }


        if (IsGamepadAvailable(0)) { //hack to speed up controller movement. 
            UpdateCameraWithGamepad(camera);
        }

        Vector3 terrainPosition = { //center the terrain around 0, 0, 0
            -terrainScale.x / 2.0f,
            0.0f,
            -terrainScale.z / 2.0f
        }; 



        HandleCameraPlayerToggle(camera, player, controlPlayer);
        UpdateCameraAndPlayer(camera, player, controlPlayer, deltaTime);
        // During render loop:

        float wave = sin(GetTime() * 0.9f) * 0.9f;  // slow, subtle vertical motion
        float animatedWaterLevel = waterHeightY + wave;
        Vector3 waterPos = {0, animatedWaterLevel, 0};
        Vector3 bottomPos = {0, waterHeightY - 100, 0};

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
        DrawDungeonBarrels(barrelModel);
        DrawDungeonPillars(pillarModel);
        //DrawDungeonCeiling(floorTile, 400.0f); // Or whatever offset looks good
        DrawDungeonCeiling(floorTile);

        DrawModel(terrainModel, terrainPosition, 1.0f, WHITE);
       
        DrawModel(waterModel, waterPos, 1.0f, WHITE); 
        DrawModel(bottomPlane, bottomPos, 1.0f, DARKBLUE); //a second plane below water plane. to prevent seeing through the world when looking down.
        DrawTrees(trees, palmTree, palm2, shadowQuad); //maybe models should be global, i think they are 
        DrawBushes(bushes, shadowQuad);
        if (!isDungeon) DrawBoat(player_boat);
        //DrawModel(floorTile, Vector3{0, 200, 0}, 0.5, WHITE);
        //DrawModel(doorWay, Vector3{0, 200, 0}, 0.5, WHITE);
        drawRaptors(camera); //sort and draw raptors
        DrawPlayer(player, camera);

        DrawBullets();
        for (auto& d : decals) {
            d.Draw(camera);
        }

        EndBlendMode();
        EndMode3D(); //////////////////EndMode3d



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
        DrawHealthBar();
        DrawStaminaBar();
        DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
        DrawText(currentInputMode == InputMode::Gamepad ? "Gamepad" : "Keyboard", 10, 30, 20, LIGHTGRAY);
        DrawText("PRESS TAB FOR FREE CAMERA", GetScreenWidth()/2 + 250, 30, 20, WHITE);

        EndDrawing();

    }

    // Cleanup
    UnloadAllResources();
    removeAllRaptors();
    RemoveAllVegetation();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
