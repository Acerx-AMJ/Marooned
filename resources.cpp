#include "resources.h"
#include "world.h"
#include "sound_manager.h"
#include "raymath.h"

RenderTexture2D sceneTexture;
Texture2D bushTex, shadowTex, raptorFront, raptorTexture, gunTexture, muzzleFlash;
Shader fogShader, skyShader, waterShader, terrainShader, shadowShader;
Model terrainModel, skyModel, waterModel, shadowQuad, palmTree, palm2, bush, boatModel, gunModel, bottomPlane, blunderbuss;
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

    //red vignette intensity over time
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "vignetteIntensity"), &vignetteIntensity, SHADER_UNIFORM_FLOAT);

// During death sequence:
    if (player.dying) {
        fadeToBlack = Clamp(player.deathTimer / 1.5f, 0.0f, 1.0f); // fade over 1.5 seconds
        SetShaderValue(fogShader, GetShaderLocation(fogShader, "fadeToBlack"), &fadeToBlack, SHADER_UNIFORM_FLOAT);
    }
    else {
        fadeToBlack = 0.0f;
        SetShaderValue(fogShader, GetShaderLocation(fogShader, "fadeToBlack"), &fadeToBlack, SHADER_UNIFORM_FLOAT);
    }
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
    terrainScale = {16000.0f, 200.0f, 16000.0f}; //4K height maps scaled to 16k looks best. 

    terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
    terrainModel = LoadModelFromMesh(terrainMesh);



    // Shaders

    //Post processing shader. AO shader + red vignette + fade to black
    fogShader = LoadShader(0, "assets/shaders/fog_postprocess.fs");
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    //SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), (float[2]){ (float)GetScreenWidth(), (float)GetScreenHeight() }, SHADER_UNIFORM_VEC2);

    //color the mesh depending on the height. 
    terrainShader = LoadShader("assets/shaders/height_color.vs", "assets/shaders/height_color.fs");
    terrainModel.materials[0].shader = terrainShader;

    // Sky
    skyShader = LoadShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    skyModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    skyModel.materials[0].shader = skyShader;

    // Shadow ////unused. 
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
    blunderbuss = LoadModel("assets/models/blunderbus.glb");
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

    SoundManager::GetInstance().LoadSound("step1", "assets/sounds/step1.ogg");
    SoundManager::GetInstance().LoadSound("step2", "assets/sounds/step2.ogg");
    SoundManager::GetInstance().LoadSound("step3", "assets/sounds/step3.ogg");
    SoundManager::GetInstance().LoadSound("step4", "assets/sounds/step4.ogg");

    SoundManager::GetInstance().LoadSound("phit1", "assets/sounds/PlayerHit1.ogg");
    SoundManager::GetInstance().LoadSound("phit2", "assets/sounds/PlayerHit2.ogg");
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
