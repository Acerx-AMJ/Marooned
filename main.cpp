#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "string"
#include "world.h"
#include "vegetation.h"
#include "player.h"

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

        camera.position = Vector3Add(player.position, (Vector3){ 0, 1.5f, 0 });
        camera.target = Vector3Add(camera.position, forward);
    } else {
        UpdateCustomCamera(&camera, deltaTime);
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


int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT); // before InitWindow
    InitWindow(800, 800, "Marooned");
    SetTargetFPS(60);

    RenderTexture2D sceneTexture = LoadRenderTexture((float)GetScreenWidth(), (float)GetScreenHeight()); //render texture
    Texture2D bushTex = LoadTexture("assets/bush.png");

    Shader fogShader = LoadShader(0,"assets/shaders/fog_postprocess.fs"); //ambient occlusion shader not fog. 
    Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &res, SHADER_UNIFORM_VEC2);
    InitPlayer(player, Vector3 {0,0,0});

    heightmap = LoadImage("assets/EyeballIsle.png");
    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE); // ensures it's 8-bit grayscale
    terrainScale = {16000.0f, 200.0f, 16000.0f};

    heightmapPixels = (unsigned char*)heightmap.data; //for iterating heightmap for tree placement. 

    Shader terrainShader = LoadShader("assets/shaders/height_color.vs", "assets/shaders/height_color.fs");

    terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
    
    Model model = LoadModelFromMesh(terrainMesh);
    model.materials[0].shader = terrainShader;

    //skybox shader
    Shader skyShader = LoadShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    Mesh skyMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skyModel = LoadModelFromMesh(skyMesh);
    skyModel.materials[0].shader = skyShader;

    Texture shadowTex = LoadTexture("assets/shadow_decal.png");
    Shader shadowShader = LoadShader("assets/shaders/shadow_decal.vs", "assets/shaders/shadow_decal.fs");
    Model shadowQuad = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1)); // 1x1 plane
    shadowQuad.materials[0].shader = shadowShader;
    SetMaterialTexture(&shadowQuad.materials[0], MATERIAL_MAP_DIFFUSE, shadowTex);

    float terrainSizeX = terrainScale.x;
    float terrainSizeZ = terrainScale.z;
    float waterHeightY = 60.0;


    Mesh waterMesh = GenMeshPlane(30000, 30000, 1, 1);

    //UploadMesh(&waterMesh, true); // reupload updated UVs

    Model waterModel = LoadModelFromMesh(waterMesh);
    //waterModel.materials[0].shader = waterShader;


    Shader waterShader = LoadShader("assets/shaders/water.vs", "assets/shaders/water.fs");
    Vector3 waterPos = { 0.0f, waterHeightY, 0.0f };
    float waterLevel = waterPos.y;
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "waterLevel"), &waterLevel, SHADER_UNIFORM_FLOAT);
    waterModel.materials[0].shader = waterShader;

    Model boat = LoadModel("assets/models/boat.glb");
    // Apply a fix rotation (e.g. -90 degrees around X to make it face forward)

    Model bush = LoadModel("assets/models/grass2.glb");


    Model palmTree = LoadModel("assets/models/bigPalmTree.glb");
    Model palm2 = LoadModel("assets/models/smallPalmTree.glb");



    //Model smallPalmTree = LoadModel("assets/models/smallPalmTree.glb");
    float treeSpacing = 150.0f;
    float minTreeSpacing = 50.0f;
    float treeHeightThreshold = terrainScale.y * TREE_HEIGHT_RATIO;
    float bushHeightThreshold = terrainScale.y * BUSH_HEIGHT_RATIO;
    
    // 🌴 Generate the trees
    std::vector<TreeInstance> trees = GenerateTrees(heightmap, heightmapPixels, terrainScale, treeSpacing, minTreeSpacing, treeHeightThreshold);

    // 🌴 Filter trees based on final height cutoff
    trees = FilterTreesAboveHeightThreshold(trees, heightmap, heightmapPixels, terrainScale, treeHeightThreshold);

    std::vector<BushInstance> bushes = GenerateBushes(heightmap, heightmapPixels, terrainScale, treeSpacing, bushHeightThreshold, bush);
    bushes = FilterBushsAboveHeightThreshold(bushes, heightmap, heightmapPixels, terrainScale, bushHeightThreshold);
    // Camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 400.0f, 0.0f };
    camera.target = (Vector3){ 10.0f, 0.0f, 10.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;


    DisableCursor();

    Vector3 boatPosition = {0, -20, -5000.0};
    float boatSpeed = 200;


    while (!WindowShouldClose()) {
        //UpdateCamera(&camera, CAMERA_FREE);
        float deltaTime = GetFrameTime();
        //UpdateCustomCamera(&camera, deltaTime);
        Vector3 camPos = camera.position;
        int camPosLoc = GetShaderLocation(terrainShader, "cameraPos");
        SetShaderValue(terrainShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);
        float time = GetTime();
        SetShaderValue(skyShader, GetShaderLocation(skyShader, "time"), &time, SHADER_UNIFORM_FLOAT);

        Vector3 forward = { 0.0f, 0.0f, 1.0f }; // facing +Z
        Vector3 velocity = Vector3Scale(forward, boatSpeed * deltaTime);
        boatPosition = Vector3Add(boatPosition, velocity);

        if (IsGamepadAvailable(0)) { //hack to speed up controller movement. 
            float lookSensitivity = 0.03f;  // ← lower = slower

            float yawInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
            float pitchInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

            float yaw = -yawInput * lookSensitivity;
            float pitch = -pitchInput * lookSensitivity;

            Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

            // Rotate around camera.up (Y axis) for yaw
            Matrix yawRot = MatrixRotate(camera.up, yaw);
            //forward = Vector3Transform(forward, yawRot);

            // Calculate right vector and rotate around it for pitch
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
            Matrix pitchRot = MatrixRotate(right, pitch);
            forward = Vector3Transform(forward, yawRot);
            forward = Vector3Transform(forward, pitchRot);

            // Update camera target based on new forward vector
            camera.target = Vector3Add(camera.position, forward);
            float boost = 1.0f; // base speed
            Vector2 stick = {
                GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
                GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)
            };

            //Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            //Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

            camera.position = Vector3Add(camera.position,
                Vector3Scale(right, stick.x * boost));
            camera.position = Vector3Add(camera.position,
                Vector3Scale(forward, -stick.y * boost));

            camera.target = Vector3Add(camera.position, forward);
        }
        
        Vector3 terrainPosition = {
            -terrainScale.x / 2.0f,
            0.0f,
            -terrainScale.z / 2.0f
        }; 

        HandleCameraPlayerToggle(camera, player, controlPlayer);
        UpdateCameraAndPlayer(camera, player, controlPlayer, deltaTime);
        // During render loop:
        float t = GetTime();
        SetShaderValue(waterShader, GetShaderLocation(waterShader, "time"), &t, SHADER_UNIFORM_FLOAT);
        DrawModel(waterModel, waterPos, 1.0f, WHITE);

        int camLoc = GetShaderLocation(waterShader, "cameraPos");
        SetShaderValue(waterShader, camLoc, &camPos, SHADER_UNIFORM_VEC3);



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
        DrawModel(model, terrainPosition, 1.0f, WHITE);
       
        DrawModel(waterModel, waterPos, 1.0f, WHITE);   

        DrawModel(boat, boatPosition, 1.0f, WHITE);
        DrawPlayer(player);
 
        DrawTrees(trees, palmTree, palm2, shadowQuad);

        DrawBushes(bushes);
        //rlEnableDepthMask();       // Re-enable depth writing
       
        rlDisableDepthTest();
        
        
        //Vector3 sunPos = {0.0f, 500.0f, 0.0f};
        //DrawSphere(sunPos, 5.0f, YELLOW);
        EndMode3D();
        EndTextureMode();

        // === POSTPROCESS TO SCREEN ===
        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(fogShader);
        DrawTextureRec(sceneTexture.texture, 
            (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
            (Vector2){0, 0}, WHITE);
        EndShaderMode();

        // === 2D Overlay (optional) ===
        DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, WHITE);

        EndDrawing();

    }

    // Cleanup
    UnloadImage(heightmap);

    UnloadModel(model);
    UnloadModel(boat);
    UnloadModel(palmTree);
    UnloadModel(bush);
    UnloadShader(terrainShader);
    UnloadShader(skyShader);
    UnloadShader(waterShader);
    CloseWindow();

    return 0;
}
