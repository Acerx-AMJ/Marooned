#include "raylib.h"
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
    if (IsKeyDown(KEY_SPACE)) inputY += 1.0f;
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
        UpdatePlayer(player, GetFrameTime(), terrainMesh);
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

void DrawBillboardPro(Camera3D camera, Texture2D texture, Vector3 worldPos, float size) {
    // Convert 3D position to 2D screen position
    Vector2 screenPos = GetWorldToScreen(worldPos, camera);

    // Define source rectangle (flipped vertically to fix upside-down texture)
    Rectangle sourceRec = {
        0.0f,
        (float)texture.height,
        (float)texture.width,
        -(float)texture.height
    };

    // Define destination rectangle (centered on screen position)
    Rectangle destRec = {
        screenPos.x - size / 2,
        screenPos.y - size,
        size,
        size
    };

    // No rotation needed because the screenPos already centers it correctly
    float rotation = 0.0f;

    // Draw with transparency
    DrawTexturePro(texture, sourceRec, destRec, { 0, 0 }, rotation, WHITE);
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

void drawWeapon3d(Camera& camera){

    // Build a rotation matrix that aligns +Z with that direction
    Matrix lookAt = MatrixLookAt(camera.position, camera.target, { 0, 1, 0 });
    Matrix gunRotation = MatrixInvert(lookAt); // Invert to use as model transform
    Quaternion gunQuat = QuaternionFromMatrix(gunRotation);

    Quaternion q = gunQuat;
    float angle = 2.0f * acosf(q.w);
    float angleDeg = angle * RAD2DEG;
    Vector3 scale = { 2.0f, 2.0f, 2.0f };
    float sinTheta = sqrtf(1.0f - q.w * q.w);
    Vector3 axis;
    if (sinTheta < 0.001f) {
        axis = { 1.0f, 0.0f, 0.0f }; // Arbitrary axis
    } else {
        axis = { q.x / sinTheta, q.y / sinTheta, q.z / sinTheta };
    }

        // Calculate camera basis vectors
    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, { 0, 1, 0 }));
    Vector3 camUp = { 0, 1, 0 };

    // Gun offset relative to camera (in camera space)
    float forwardOffset = 80.0f;
    float sideOffset = 20.0f;
    float verticalOffset = -30.0f;

    // Combine into world space position
    Vector3 gunPos = camera.position;
    gunPos = Vector3Add(gunPos, Vector3Scale(camForward, forwardOffset));
    gunPos = Vector3Add(gunPos, Vector3Scale(camRight, sideOffset));
    gunPos = Vector3Add(gunPos, Vector3Scale(camUp, verticalOffset));



    DrawModelEx(gunModel, gunPos, axis, angleDeg, scale, WHITE);

}

void drawWeapon(){
    Rectangle source = { 0, 0, 1024, 1024}; 
    Rectangle dest = {
        GetScreenWidth()* 0.5f,              
        GetScreenHeight() * 0.6f,            
        1024.0f,                             
        1024.0f                              
    };
    Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f }; // center the sprite

    DrawTexturePro(gunTexture, source, dest, origin, 0.0f, WHITE);
}


int main() {
    //SetConfigFlags(FLAG_MSAA_4X_HINT); //anti aliasing, I see no difference. 
    InitWindow(1600, 900, "Marooned");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadAllResources();
    generateVegetation();
   
    InitPlayer(player, Vector3 {0,0,0});
    generateRaptors(5, player.position, 6000);
    

    // Camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 400.0f, 0.0f };
    camera.target = (Vector3){ 10.0f, 0.0f, 10.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    DisableCursor();
    InitBoat(player_boat, boatPosition);



    while (!WindowShouldClose()) {
        UpdateInputMode(); //handle both gamepad and keyboard/mouse
        debugControls(); //press P to remove all vegetation
        float deltaTime = GetFrameTime();
        //UpdateCustomCamera(&camera, deltaTime);
        Vector3 camPos = camera.position;
        int camPosLoc = GetShaderLocation(terrainShader, "cameraPos");
        SetShaderValue(terrainShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);
        float time = GetTime();
        SetShaderValue(skyShader, GetShaderLocation(skyShader, "time"), &time, SHADER_UNIFORM_FLOAT);
        
        Vector3 forward = { 0.0f, 0.0f, 1.0f }; // facing +Z
        //Vector3 velocity = Vector3Scale(forward, boatSpeed * deltaTime);
        //boatPosition = Vector3Add(boatPosition, velocity);

        if (IsGamepadAvailable(0)) { //hack to speed up controller movement. 
            UpdateCameraWithGamepad(camera);
        }

        Vector3 terrainPosition = {
            -terrainScale.x / 2.0f,
            0.0f,
            -terrainScale.z / 2.0f
        }; 

        float wave = sin(GetTime() * 0.9f) * 0.9f;  // slow, subtle vertical motion
        float animatedWaterLevel = waterHeightY + wave;

        HandleCameraPlayerToggle(camera, player, controlPlayer);
        UpdateCameraAndPlayer(camera, player, controlPlayer, deltaTime);
        // During render loop:
        float t = GetTime();
        Vector3 waterPos = {0, animatedWaterLevel, 0};
        SetShaderValue(waterShader, GetShaderLocation(waterShader, "time"), &t, SHADER_UNIFORM_FLOAT);
        DrawModel(waterModel, waterPos, 1.0f, WHITE);

        int camLoc = GetShaderLocation(waterShader, "cameraPos");
        SetShaderValue(waterShader, camLoc, &camPos, SHADER_UNIFORM_VEC3);
        UpdateBoat(player_boat, deltaTime);
    
        for (Character& raptor : raptors) {
            raptor.Update(deltaTime, player.position, heightmap, terrainScale, raptorPtrs);
        }
        // === RENDER TO TEXTURE ===
        BeginTextureMode(sceneTexture);
        ClearBackground(SKYBLUE);

        BeginCustom3D(camera, 50000.0f);
        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(skyModel, camera.position, 10000.0f, WHITE);
        rlEnableDepthMask(); 
        rlEnableDepthTest();
        rlSetBlendMode(BLEND_ALPHA);
        rlEnableColorBlend();
        DrawModel(terrainModel, terrainPosition, 1.0f, WHITE);
       
        DrawModel(waterModel, waterPos, 1.0f, WHITE);   
        //DrawModel(gunModel, Vector3{0, 300, 0}, 1.0f, WHITE);
        DrawBoat(player_boat);


        drawRaptors(camera); //sort and draw raptors
        DrawPlayer(player, camera);

        DrawTrees(trees, palmTree, palm2, shadowQuad);

        DrawBushes(bushes, shadowQuad);

    
        EndBlendMode();
        EndMode3D();

        rlDisableDepthTest();
        //DrawTexture(gunTexture, GetScreenWidth() * 0.55f, GetScreenHeight() * 0.7f, WHITE);

        EndTextureMode();

        // === POSTPROCESS TO SCREEN ===
        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(fogShader);
        DrawTextureRec(sceneTexture.texture, 
            (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
            (Vector2){0, 0}, WHITE);
        EndShaderMode();

        // if (controlPlayer){
        //     drawWeapon();
        // }



        DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);
        DrawText(currentInputMode == InputMode::Gamepad ? "Gamepad" : "Keyboard", 10, 30, 20, LIGHTGRAY);

        EndDrawing();

    }

    // Cleanup
    UnloadAllResources();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
