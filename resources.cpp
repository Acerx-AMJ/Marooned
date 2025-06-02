#include "resources.h"
#include "world.h"
#include "sound_manager.h"

RenderTexture2D sceneTexture;
Texture2D bushTex, shadowTex, raptorFront, raptorTexture, gunTexture, muzzleFlash;
Shader fogShader, skyShader, waterShader, terrainShader, shadowShader;
Model terrainModel, skyModel, waterModel, shadowQuad, palmTree, palm2, bush, boatModel, gunModel, bottomPlane;
Image heightmap;
Mesh terrainMesh;
Sound musket;

Vector3 terrainScale;

Vector2 screenResolution;

void UpdateShaders(Camera& camera){
    float t = GetTime();
    Vector3 camPos = camera.position;
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "time"), &t, SHADER_UNIFORM_FLOAT);

    int camLoc = GetShaderLocation(waterShader, "cameraPos");
    SetShaderValue(waterShader, camLoc, &camPos, SHADER_UNIFORM_VEC3);

 
    int camPosLoc = GetShaderLocation(terrainShader, "cameraPos");
    SetShaderValue(terrainShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);
    //float time = GetTime();
    SetShaderValue(skyShader, GetShaderLocation(skyShader, "time"), &t, SHADER_UNIFORM_FLOAT);
}

void LoadAllResources() {
    screenResolution = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    sceneTexture = LoadRenderTexture((int)screenResolution.x, (int)screenResolution.y);

    raptorTexture = LoadTexture("assets/sprites/raptorSheet.png");
    gunTexture = LoadTexture("assets/sprites/flintlock.png");
    gunModel = LoadModel("assets/models/blunderbus.glb");
    muzzleFlash = LoadTexture("assets/sprites/muzzleFlash.png");

    // Heightmap //TODO: refactor this into level switching, an array of heightmaps. A menu to increase or decrease the index. 
    heightmap = LoadImage("assets/MiddleIsland.png"); ///////////////////////// current map
    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    terrainScale = {16000.0f, 200.0f, 16000.0f};

    terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
    terrainModel = LoadModelFromMesh(terrainMesh);

    // Shaders
    fogShader = LoadShader(0, "assets/shaders/fog_postprocess.fs");
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);

    terrainShader = LoadShader("assets/shaders/height_color.vs", "assets/shaders/height_color.fs");
    terrainModel.materials[0].shader = terrainShader;

    // Sky
    skyShader = LoadShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    skyModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    skyModel.materials[0].shader = skyShader;

    // Shadow
    shadowTex = LoadTexture("assets/shadow_decal.png");
    shadowShader = LoadShader("assets/shaders/shadow_decal.vs", "assets/shaders/shadow_decal.fs");
    shadowQuad = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
    shadowQuad.materials[0].shader = shadowShader;
    SetMaterialTexture(&shadowQuad.materials[0], MATERIAL_MAP_DIFFUSE, shadowTex);

    // Water
    waterShader = LoadShader("assets/shaders/water.vs", "assets/shaders/water.fs");
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "waterLevel"), &waterHeightY, SHADER_UNIFORM_FLOAT);
    waterModel = LoadModelFromMesh(GenMeshPlane(30000, 30000, 1, 1));
    bottomPlane = LoadModelFromMesh(GenMeshPlane(30000, 30000, 1, 1));
    waterModel.materials[0].shader = waterShader;
    bottomPlane.materials[0].shader = waterShader;

    // Models
    
    palmTree = LoadModel("assets/models/bigPalmTree.glb");
    palm2 = LoadModel("assets/models/smallPalmTree.glb");
    bush = LoadModel("assets/models/grass2.glb");
    boatModel = LoadModel("assets/models/boat.glb");

    bushTex = LoadTexture("assets/bush.png");

    //Sounds
    SoundManager::GetInstance().LoadSound("dinoHit", "assets/sounds/dinoHit.ogg");
    SoundManager::GetInstance().LoadSound("dinoDeath", "assets/sounds/dinoDeath.ogg");
    SoundManager::GetInstance().LoadSound("dinoTweet", "assets/sounds/dino1.ogg");
    SoundManager::GetInstance().LoadSound("dinoTarget", "assets/sounds/dino2.ogg");
    SoundManager::GetInstance().LoadSound("dinoTweet2", "assets/sounds/dino3.ogg");
    SoundManager::GetInstance().LoadSound("dinoBite", "assets/sounds/bite.ogg");
    SoundManager::GetInstance().LoadSound("reload", "assets/sounds/reload.ogg");
    SoundManager::GetInstance().LoadSound("shotgun", "assets/sounds/shotgun.ogg");
}

void UnloadAllResources() {
    UnloadRenderTexture(sceneTexture);
    UnloadTexture(bushTex);
    UnloadTexture(shadowTex);
    UnloadTexture(raptorTexture);
    UnloadTexture(gunTexture);
    UnloadShader(fogShader);
    UnloadShader(skyShader);
    UnloadShader(waterShader);
    UnloadShader(terrainShader);
    UnloadShader(shadowShader);
    UnloadModel(skyModel);
    UnloadModel(waterModel);
    UnloadModel(shadowQuad);
    UnloadModel(palmTree);
    UnloadModel(palm2);
    UnloadModel(bush);
    UnloadModel(boatModel);
    UnloadModel(gunModel);
    UnloadImage(heightmap);
    UnloadMesh(terrainMesh);
    

}
