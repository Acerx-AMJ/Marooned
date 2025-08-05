#include "resources.h"
#include "world.h"
#include "sound_manager.h"
#include "raymath.h"
#include <iostream>
#include "rlgl.h"
#include "custom_rendertexture.h"
#include "dungeonGeneration.h"
#include "rlgl.h"
#include "resourceManager.h"
//Add maps and a get function to resources manager instead of global variable hell. 

Model terrainModel;

Image heightmap;
Mesh terrainMesh;

// int sceneTextureLoc;
// int sceneDepthLoc; 
Vector3 terrainScale = {16000.0f, 200.0f, 16000.0f}; //very large x and z, 

Vector2 screenResolution; //global shader resolution
#define MAX_MATERIAL_MAPS 12
//auto& R = ResourceManager::Get();

void LoadAllResources() {
    screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };
    //slowly get rid of the globals, When you need a texture, use the resourceManager::Get() function. 
    R.LoadRenderTexture("sceneTexture", (int)screenResolution.x, (int)screenResolution.y);
    R.LoadRenderTexture("postProcessTexture", (int)screenResolution.x,(int) screenResolution.y);

    //normal textures. Save textures to an unordered map, look up by string, call R.GetTexture("name") to access texture
    R.LoadTexture("raptorTexture",    "assets/sprites/raptorSheet.png");
    R.LoadTexture("skeletonSheet",    "assets/sprites/skeletonSheet.png");
    R.LoadTexture("muzzleFlash",      "assets/sprites/muzzleFlash.png");
    R.LoadTexture("backDrop",         "assets/screenshots/dungeon1.png");
    R.LoadTexture("smokeSheet",       "assets/sprites/smokeSheet.png");
    R.LoadTexture("bloodSheet",       "assets/sprites/bloodSheet.png");
    R.LoadTexture("doorTexture",      "assets/sprites/door.png");
    R.LoadTexture("healthPotTexture", "assets/sprites/Healthpot.png");
    R.LoadTexture("keyTexture",       "assets/sprites/key.png");
    R.LoadTexture("swordBloody",      "assets/textures/swordBloody.png");
    R.LoadTexture("swordClean",       "assets/textures/swordClean.png");
    R.LoadTexture("fireSheet",        "assets/sprites/fireSheet.png");
    R.LoadTexture("pirateSheet",      "assets/sprites/pirateSheet.png");
    R.LoadTexture("coinTexture",      "assets/sprites/coin.png");
    R.LoadTexture("spiderSheet",      "assets/sprites/spiderSheet.png");
    R.LoadTexture("spiderWebTexture", "assets/sprites/spiderWeb.png");
    R.LoadTexture("brokeWebTexture",  "assets/sprites/brokeWeb.png");
    R.LoadTexture("explosionSheet",   "assets/sprites/explosionSheet.png");
    R.LoadTexture("manaPotion",       "assets/sprites/manaPotion.png");
    R.LoadTexture("fireIcon",         "assets/sprites/fireIcon.png");
    R.LoadTexture("iceIcon",          "assets/sprites/iceIcon.png");
    R.LoadTexture("shadowTex",        "assets/textures/shadow_decal.png");


    // Models (registering with string keys)
    R.LoadModel("palmTree",       "assets/models/bigPalmTree.glb");
    R.LoadModel("palm2",          "assets/models/smallPalmTree.glb");
    R.LoadModel("bush",           "assets/models/grass(stripped).glb");
    R.LoadModel("boatModel",      "assets/models/boat.glb");
    R.LoadModel("blunderbuss",    "assets/models/blunderbus.glb");
    R.LoadModel("floorTile",      "assets/models/floorTile.glb");
    R.LoadModel("doorWay",        "assets/models/doorWay.glb");
    R.LoadModel("wall",           "assets/models/wall.glb");
    R.LoadModel("barrelModel",    "assets/models/barrel.glb");
    R.LoadModel("swordModel",     "assets/models/sword.glb");
    R.LoadModel("lampModel",      "assets/models/lamp.glb");
    R.LoadModel("brokeBarrel",    "assets/models/brokeBarrel.glb");
    R.LoadModel("chestModel",     "assets/models/chest.glb");
    R.LoadModel("staffModel",     "assets/models/staff.glb");
    R.LoadModel("fireballModel",  "assets/models/fireball.glb");
    R.LoadModel("iceballModel",   "assets/models/iceBall.glb");

    //generated models
    R.LoadModelFromMesh("skyModel", GenMeshCube(1.0f, 1.0f, 1.0f));
    R.LoadModelFromMesh("waterModel",GenMeshPlane(30000, 30000, 1, 1));
    R.LoadModelFromMesh("bottomPlane",GenMeshPlane(30000, 30000, 1, 1));
    R.LoadModelFromMesh("shadowQuad",GenMeshPlane(1.0f, 1.0f, 1, 1));

    //shaders
    R.LoadShader("terrainShader", "assets/shaders/height_color.vs",  "assets/shaders/height_color.fs");
    R.LoadShader("fogShader",     /*vsPath=*/"",                    "assets/shaders/fog_postprocess.fs");
    R.LoadShader("shadowShader",  "assets/shaders/shadow_decal.vs", "assets/shaders/shadow_decal.fs");
    R.LoadShader("skyShader",     "assets/shaders/skybox.vs",       "assets/shaders/skybox.fs");
    R.LoadShader("waterShader",   "assets/shaders/water.vs",        "assets/shaders/water.fs");
    R.LoadShader("bloomShader",   /*vsPath=*/"",                    "assets/shaders/bloom.fs");


    // set shader uniforms
    Shader& fogShader = R.GetShader("fogShader");
    Shader& bloomShader = R.GetShader("bloomShader");
    Shader& shadowShader = R.GetShader("shadowShader");
    Shader& waterShader = R.GetShader("waterShader");

    // Sky
    
    R.GetModel("skyModel").materials[0].shader = R.GetShader("skyShader");

    // Shadow //
    Model& shadowQuad = R.GetModel("shadowQuad");//LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
    shadowQuad.materials[0].shader = shadowShader;
    SetMaterialTexture(&shadowQuad.materials[0], MATERIAL_MAP_DIFFUSE, R.GetTexture("shadowTex"));

    // Water
    
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "waterLevel"), &waterHeightY, SHADER_UNIFORM_FLOAT);
    R.GetModel("waterModel").materials[0].shader = waterShader;
    R.GetModel("bottomPlane").materials[0].shader = waterShader;

    //bloom post process. 
    float bloomStrengthValue = 0.3f;
    float bloomColor[3] = { 1.0f, 0.0f, 1.0f }; // Slight purple tint
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomColor"), bloomColor, SHADER_UNIFORM_VEC3);


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

    SoundManager::GetInstance().LoadSound("doorOpen", "assets/sounds/doorOpen.ogg");
    SoundManager::GetInstance().LoadSound("doorClose", "assets/sounds/doorCLose.ogg");
    SoundManager::GetInstance().LoadSound("swipe1", "assets/sounds/swipe1.ogg");
    SoundManager::GetInstance().LoadSound("swipe2", "assets/sounds/swipe2.ogg");
    SoundManager::GetInstance().LoadSound("swipe3", "assets/sounds/swipe3.ogg");
    SoundManager::GetInstance().LoadSound("swordHit", "assets/sounds/swordHit.ogg");
    SoundManager::GetInstance().LoadSound("swordBlock", "assets/sounds/swordBlock.ogg");
    SoundManager::GetInstance().LoadSound("swordBlock2", "assets/sounds/swordBlock2.ogg");
    SoundManager::GetInstance().LoadSound("bones", "assets/sounds/bones.ogg");
    SoundManager::GetInstance().LoadSound("bones2", "assets/sounds/bones2.ogg");
    SoundManager::GetInstance().LoadSound("gulp", "assets/sounds/gulp.ogg");
    SoundManager::GetInstance().LoadSound("clink", "assets/sounds/clink.ogg");
    SoundManager::GetInstance().LoadSound("lockedDoor", "assets/sounds/lockedDoor.ogg");
    SoundManager::GetInstance().LoadSound("unlock", "assets/sounds/unlock.ogg");
    SoundManager::GetInstance().LoadSound("key", "assets/sounds/KeyGet.ogg");
    SoundManager::GetInstance().LoadSound("barrelBreak", "assets/sounds/barrelBreak.ogg");
    SoundManager::GetInstance().LoadSound("musket", "assets/sounds/musket.ogg");
    SoundManager::GetInstance().LoadSound("chestOpen", "assets/sounds/chestOpen.ogg");
    SoundManager::GetInstance().LoadSound("spiderBite1", "assets/sounds/spiderBite1.ogg");
    SoundManager::GetInstance().LoadSound("spiderBite2", "assets/sounds/spiderBite2.ogg");
    SoundManager::GetInstance().LoadSound("spiderDeath", "assets/sounds/spiderDeath.ogg");
    SoundManager::GetInstance().LoadSound("flame1", "assets/sounds/flame1.ogg");
    SoundManager::GetInstance().LoadSound("flame2", "assets/sounds/flame2.ogg");
    SoundManager::GetInstance().LoadSound("explosion", "assets/sounds/explosion.ogg");
    SoundManager::GetInstance().LoadSound("staffHit", "assets/sounds/staffHit.ogg");
    SoundManager::GetInstance().LoadSound("iceMagic", "assets/sounds/iceMagic.ogg");
    //music (ambience)
    SoundManager::GetInstance().LoadMusic("dungeonAir", "assets/sounds/dungeonAir.ogg");
    SoundManager::GetInstance().LoadMusic("jungleAmbience", "assets/sounds/jungleSounds.ogg");

}

void UpdateShaders(Camera& camera){
    Shader& waterShader = R.GetShader("waterShader");
    Shader& skyShader = R.GetShader("skyShader");
    Shader& terrainShader = R.GetShader("terrainShader");
    Shader& fogShader = R.GetShader("fogShader");
    Shader& bloomShader = R.GetShader("bloomShader");

    float t = GetTime();
    Vector3 camPos = camera.position;
    int dungeonFlag = isDungeon ? 1 : 0;
    int isDungeonLoc = GetShaderLocation(skyShader, "isDungeon");

    int camLoc = GetShaderLocation(waterShader, "cameraPos");
    int camPosLoc = GetShaderLocation(terrainShader, "cameraPos");
    SetShaderValue(waterShader, camLoc, &camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "time"), &t, SHADER_UNIFORM_FLOAT);

    SetShaderValue(terrainShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);

    SetShaderValue(skyShader, GetShaderLocation(skyShader, "time"), &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, isDungeonLoc, &dungeonFlag, SHADER_UNIFORM_INT);

    //red vignette intensity over time
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "vignetteIntensity"), &vignetteIntensity, SHADER_UNIFORM_FLOAT);

    //dungeonDarkness
    float dungeonDarkness = 0.0f;//it darkens the gun model as well, so go easy. negative number brightens it. 
    float dungeonContrast = 1.2f; //makes darks darker. 

    float aaStrengthValue = 0.5f; //fake antialiasing strength

    int isDungeonVal = isDungeon ? 1 : 0;
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "isDungeon"), &isDungeonVal, SHADER_UNIFORM_INT);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "dungeonDarkness"), &dungeonDarkness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "dungeonContrast"), &dungeonContrast, SHADER_UNIFORM_FLOAT);

    //check every frame
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "aaStrength"), &aaStrengthValue, SHADER_UNIFORM_FLOAT);

}

void UnloadAllResources() {
    ResourceManager::Get().UnloadAll();
    SoundManager::GetInstance().UnloadAll();
}
