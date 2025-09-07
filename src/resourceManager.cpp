
#include "resourceManager.h"
#include "world.h"
#include "lighting.h"

ResourceManager* ResourceManager::_instance = nullptr;

ResourceManager& ResourceManager::Get() {
    if (!_instance) _instance = new ResourceManager();
    return *_instance;
}

// Texture
Texture2D& ResourceManager::LoadTexture(const std::string& name, const std::string& path) {
    auto it = _textures.find(name);
    if (it != _textures.end()) return it->second;
    Texture2D tex = ::LoadTexture(path.c_str());
    _textures.emplace(name, tex);
    return _textures[name];
}
Texture2D& ResourceManager::GetTexture(const std::string& name) const {
    auto it = _textures.find(name);
    if (it == _textures.end()) throw std::runtime_error("Texture not found: " + name);
    return const_cast<Texture2D&>(it->second);
}

// Model
Model& ResourceManager::LoadModel(const std::string& name, const std::string& path) {
    auto it = _models.find(name);
    if (it != _models.end()) return it->second;
    Model m = ::LoadModel(path.c_str());
    _models.emplace(name, m);
    return _models[name];
}
Model& ResourceManager::LoadModelFromMesh(const std::string& name, const Mesh& mesh) {
    auto it = _models.find(name);
    if (it != _models.end()) return it->second;
    Model m = ::LoadModelFromMesh(mesh);
    _models.emplace(name, m);
    return _models[name];
}
Model& ResourceManager::GetModel(const std::string& name) const {
    auto it = _models.find(name);
    if (it == _models.end()) throw std::runtime_error("Model not found: " + name);
    return const_cast<Model&>(it->second);
}

// Shader
Shader& ResourceManager::LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
    auto it = _shaders.find(name);
    if (it != _shaders.end()) return it->second;
    Shader s = ::LoadShader(vsPath.c_str(), fsPath.c_str());
    _shaders.emplace(name, s);
    return _shaders[name];
}
Shader& ResourceManager::GetShader(const std::string& name) const {
    auto it = _shaders.find(name);
    if (it == _shaders.end()) throw std::runtime_error("Shader not found: " + name);
    return const_cast<Shader&>(it->second);
}


// RenderTexture
RenderTexture2D& ResourceManager::LoadRenderTexture(const std::string& name, int w, int h) {
    auto it = _renderTextures.find(name);
    if (it != _renderTextures.end()) return it->second;
    RenderTexture2D rt = ::LoadRenderTexture(w, h);
    _renderTextures.emplace(name, rt);
    return _renderTextures[name];
}
RenderTexture2D& ResourceManager::GetRenderTexture(const std::string& name) const {
    auto it = _renderTextures.find(name);
    if (it == _renderTextures.end()) throw std::runtime_error("RenderTexture not found: " + name);
    return const_cast<RenderTexture2D&>(it->second);
}

void ResourceManager::LoadAllResources() {
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };
    //render textures
    R.LoadRenderTexture("sceneTexture", (int)screenResolution.x, (int)screenResolution.y);
    R.LoadRenderTexture("postProcessTexture", (int)screenResolution.x,(int) screenResolution.y);

    //Resources are saved to unordered maps, with a string key. Get a resource by calling R.GetModel("Blunderbuss") for example. 
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
    R.LoadTexture("ghostSheet",       "assets/sprites/ghostSheet.png");
    R.LoadTexture("magicAttackSheet", "assets/sprites/magicAttackSheet.png");


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
    R.LoadModel("campFire",       "assets/models/campFire.glb");
    R.LoadModel("stonePillar",    "assets/models/stonePillar.glb");
    R.LoadModel("wallSegment",    "assets/models/wallSegment.glb");
    R.LoadModel("floorTileGray",  "assets/models/floorTileGray.glb");
    R.LoadModel("doorWayGray",    "assets/models/doorWayGray.glb");

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
    R.LoadShader("cutoutShader",                        "",         "assets/shaders/leaf_cutout.fs");
    R.LoadShader("lightingShader", "assets/shaders/lighting_baked_xz.vs", "assets/shaders/lighting_baked_xz.fs");
}



void ResourceManager::SetShaderValues(){
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };
    // set shaders values
    Shader& fogShader = R.GetShader("fogShader");
    Shader& bloomShader = R.GetShader("bloomShader");
    Shader& shadowShader = R.GetShader("shadowShader");
    Shader& waterShader = R.GetShader("waterShader");

    // Sky
    //apply skyShader to sky model
    R.GetModel("skyModel").materials[0].shader = R.GetShader("skyShader");

    // Shadow //Shadow decals beneath trees. 
    Model& shadowQuad = R.GetModel("shadowQuad");
    shadowQuad.materials[0].shader = shadowShader;
    SetMaterialTexture(&shadowQuad.materials[0], MATERIAL_MAP_DIFFUSE, R.GetTexture("shadowTex"));

    // Water
    
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "waterLevel"), &waterHeightY, SHADER_UNIFORM_FLOAT);
    R.GetModel("waterModel").materials[0].shader = waterShader;
    R.GetModel("bottomPlane").materials[0].shader = waterShader;

    //bloom post process. 
    bloomStrengthValue = 0.0f;
    float bloomColor[3] = { 1.0f, 0.0f, 1.0f };  
    float aaStrengthValue = 0.0f; //fake antialiasing strength, makes it grayer

    int locSat = GetShaderLocation(bloomShader, "uSaturation");
    float sat = 1.1f; // try 1.05–1.25
    SetShaderValue(bloomShader, locSat, &sat, SHADER_UNIFORM_FLOAT);

    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomColor"), bloomColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "aaStrength"), &aaStrengthValue, SHADER_UNIFORM_FLOAT);

    
    int locCutoff = GetShaderLocation(R.GetShader("cutoutShader"), "alphaCutoff");
    float cutoff = 0.5f;
    SetShaderValue(R.GetShader("cutoutShader"), locCutoff, &cutoff, SHADER_UNIFORM_FLOAT);
    Model& palm = R.GetModel("palmTree");
    Model& palm2 = R.GetModel("palm2");
   
    for (int m = 0; m < palm.materialCount; ++m) {
        
        palm.materials[m].shader = R.GetShader("cutoutShader");
        palm2.materials[m].shader = R.GetShader("cutoutShader");  
    }

    Shader lightingShader = R.GetShader("lightingShader");
    Model& floorModel = R.GetModel("floorTileGray");
    Model& wallModel = R.GetModel("wallSegment");
    Model& doorwayModel = R.GetModel("doorWayGray");

    for (int i=0; i < wallModel.materialCount; i++){
        wallModel.materials[i].shader = lightingShader;
    }

    for (int i=0; i < doorwayModel.materialCount; i++){
        doorwayModel.materials[i].shader = lightingShader;
    }

    for (int i=0; i<floorModel.materialCount; ++i)
        floorModel.materials[i].shader = lightingShader;


    Shader use = floorModel.materials[0].shader; // guaranteed the one actually used
    int locGrid   = GetShaderLocation(use, "gridBounds");
    int locDynTex = GetShaderLocation(use, "dynamicGridTex");
    int locDynStr = GetShaderLocation(use, "dynStrength");
    int locAmb    = GetShaderLocation(use, "ambientBoost");


    float grid[4] = { gDynamic.minX, gDynamic.minZ,
                    gDynamic.sizeX ? 1.0f/gDynamic.sizeX : 0.0f,
                    gDynamic.sizeZ ? 1.0f/gDynamic.sizeZ : 0.0f };
    if (locGrid   >= 0) SetShaderValue(use, locGrid, grid, SHADER_UNIFORM_VEC4);

    float dynStrength = 0.8f, ambientBoost = 0.25f;
    if (locDynStr >= 0) SetShaderValue(use, locDynStr, &dynStrength, SHADER_UNIFORM_FLOAT);
    if (locAmb    >= 0) SetShaderValue(use, locAmb,    &ambientBoost, SHADER_UNIFORM_FLOAT);

    if (locDynTex >= 0) SetShaderValueTexture(use, locDynTex, gDynamic.tex);


}

void ResourceManager::UpdateShaders(Camera& camera){
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };

    Shader& waterShader = R.GetShader("waterShader");
    Shader& skyShader = R.GetShader("skyShader");
    Shader& terrainShader = R.GetShader("terrainShader");
    Shader& fogShader = R.GetShader("fogShader");
    Shader& bloomShader = R.GetShader("bloomShader");

    Vector3 camPos = camera.position;

    float t = GetTime();
    int dungeonFlag = isDungeon ? 1 : 0;
    int isDungeonLoc = GetShaderLocation(skyShader, "isDungeon");
    int camLoc = GetShaderLocation(waterShader, "cameraPos");
    int camPosLoc = GetShaderLocation(terrainShader, "cameraPos");

    //water shader needs cameraPos for reasons. 
    SetShaderValue(waterShader, camLoc, &camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "time"), &t, SHADER_UNIFORM_FLOAT);
    
    //distance based desaturation on terrain needs camera pos
    SetShaderValue(terrainShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);

    //animate sky needs elapsed time
    SetShaderValue(skyShader, GetShaderLocation(skyShader, "time"), &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, isDungeonLoc, &dungeonFlag, SHADER_UNIFORM_INT);

    //red vignette intensity over time
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "vignetteIntensity"), &vignetteIntensity, SHADER_UNIFORM_FLOAT);

    //dungeonDarkness
    float dungeonDarkness = -0.1f;//it darkens the gun model as well, so go easy. negative number brightens it. 
    float dungeonContrast = 1.2f; //makes darks darker. 


    int isDungeonVal = isDungeon ? 1 : 0;
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "isDungeon"), &isDungeonVal, SHADER_UNIFORM_INT);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "dungeonDarkness"), &dungeonDarkness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "dungeonContrast"), &dungeonContrast, SHADER_UNIFORM_FLOAT);


}


void ResourceManager::UnloadAll() {
    UnloadContainer(_textures,        ::UnloadTexture);
    UnloadContainer(_models,          ::UnloadModel);
    UnloadContainer(_shaders,         ::UnloadShader);
    UnloadContainer(_renderTextures,  ::UnloadRenderTexture);
}

ResourceManager::~ResourceManager() {
    UnloadAll();
}

