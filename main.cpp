#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "string"
#include "terrain_chunks.h"
#include "vegetation.h"

#define GLSL_VERSION 330


void UpdateCustomCamera(Camera3D* camera, float deltaTime) {
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

void BeginCustom3D(Camera3D camera, float farClip) {
    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    //Matrix proj = MatrixPerspective(DEG2RAD * camera.fovy, (float)GetScreenWidth() / GetScreenHeight(), 0.1f, farClip);
    Matrix proj = MatrixPerspective(DEG2RAD * camera.fovy, (float)GetScreenWidth() / (float)GetScreenHeight(), 10.0f, 18000.0f);

    rlMultMatrixf(MatrixToFloat(proj));

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(view));
}


int main() {
    InitWindow(800, 800, "Rolling Hills");
    SetTargetFPS(60);

    RenderTexture2D sceneTexture = LoadRenderTexture((float)GetScreenWidth(), (float)GetScreenHeight()); //render texture
    

    Shader fogShader = LoadShader(0,"assets/shaders/fog_postprocess.fs");
    Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &res, SHADER_UNIFORM_VEC2);
    

    Image heightmap = LoadImage("assets/donutIsle.png");
    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE); // ensures it's 8-bit grayscale
    Vector3 terrainScale = {16000.0f, 300.0f, 16000.0f};
    unsigned char* pixels = (unsigned char*)heightmap.data;       // now we can index safely

    Shader terrainShader = LoadShader("assets/shaders/height_color.vs", "assets/shaders/height_color.fs");

    Mesh mesh = GenMeshHeightmap(heightmap, terrainScale);

    Model model = LoadModelFromMesh(mesh);
    model.materials[0].shader = terrainShader;

    //skybox shader
    Shader skyShader = LoadShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    Mesh skyMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skyModel = LoadModelFromMesh(skyMesh);
    skyModel.materials[0].shader = skyShader;


    float terrainSizeX = terrainScale.x;
    float terrainSizeZ = terrainScale.z;
    float waterHeightY = 30.0;


    Mesh waterMesh = GenMeshPlane(30000, 30000, 1, 1);

    //UploadMesh(&waterMesh, true); // reupload updated UVs

    Model waterModel = LoadModelFromMesh(waterMesh);
    //waterModel.materials[0].shader = waterShader;


    Vector3 waterPos = { 0.0f, waterHeightY, 0.0f };

    Shader waterShader = LoadShader("assets/shaders/water.vs", "assets/shaders/water.fs");

    waterModel.materials[0].shader = waterShader;
    Model boat = LoadModel("assets/models/boat.glb");
    // Apply a fix rotation (e.g. -90 degrees around X to make it face forward)


    Model palmTree = LoadModel("assets/models/bigPalmTree.glb");
    //Model smallPalmTree = LoadModel("assets/models/smallPalmTree.glb");
    float treeSpacing = 150.0f;
    float minTreeSpacing = 50.0f;
    float treeHeightThreshold = 150.0f;
    
    // 🌴 Generate the trees
    std::vector<TreeInstance> trees = GenerateTrees(heightmap, pixels, terrainScale, treeSpacing, minTreeSpacing, treeHeightThreshold);

    // 🌴 Filter trees based on final height cutoff
    trees = FilterTreesAboveHeightThreshold(trees, heightmap, pixels, terrainScale, treeHeightThreshold);


    // Camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 400.0f, 0.0f };
    camera.target = (Vector3){ 10.0f, 0.0f, 10.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;


    DisableCursor();

    Vector3 boatPosition = {0, -60, -5000.0};
    float boatSpeed = 200;


    while (!WindowShouldClose()) {
        //UpdateCamera(&camera, CAMERA_FREE);
        float deltaTime = GetFrameTime();
        UpdateCustomCamera(&camera, deltaTime);
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

        // During render loop:
        float t = GetTime();
        SetShaderValue(waterShader, GetShaderLocation(waterShader, "time"), &t, SHADER_UNIFORM_FLOAT);
        DrawModel(waterModel, waterPos, 1.0f, WHITE);

        int camLoc = GetShaderLocation(waterShader, "cameraPos");
        SetShaderValue(waterShader, camLoc, &camPos, SHADER_UNIFORM_VEC3);

        // === RENDER TO TEXTURE ===
        BeginTextureMode(sceneTexture);
        ClearBackground(SKYBLUE);

        BeginCustom3D(camera, 10000.0f);
        rlDisableBackfaceCulling(); rlDisableDepthMask(); rlDisableDepthTest();
        DrawModel(skyModel, camera.position, 10000.0f, WHITE);
        rlEnableDepthMask(); 
        rlEnableBackfaceCulling();

        rlEnableDepthTest();
        rlEnableColorBlend();
        //DrawModel(redTest, (Vector3){ 0, 600, 0 }, 1.0f, WHITE);
  
        DrawModel(model, terrainPosition, 1.0f, WHITE);
       
        DrawModel(waterModel, waterPos, 1.0f, WHITE);   

        DrawModel(boat, boatPosition, 1.0f, WHITE);

        for (const auto& tree : trees) {
            Vector3 pos = tree.position;
            pos.y += tree.yOffset;
            pos.x += tree.xOffset;
            pos.z += tree.zOffset;
            DrawModelEx(palmTree, pos, { 0, 1, 0 }, tree.rotationY,
                        { tree.scale, tree.scale, tree.scale }, WHITE);
        }

        //DrawTerrainChunks(chunks);
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
    //UnloadTexture(heightmapTex);
    UnloadModel(model);
    UnloadShader(shader);
    CloseWindow();

    return 0;
}
