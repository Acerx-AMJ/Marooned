#include "util/resourceManager.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include "world/world.h"
#include "render/lighting.h"

// Constructors

ResourceManager::~ResourceManager() {
    UnloadAll();
}

ResourceManager& ResourceManager::Get() {
    static ResourceManager instance;
    return instance;
}

// Load, get functions

Texture2D& ResourceManager::LoadTexture(const std::string& name, const std::string& path) {
    auto it = textures.find(name);
    if (it != textures.end()) return it->second;

    Texture2D tex = ::LoadTexture(path.c_str());
    if (tex.id == 0) {
        std::cerr << "Failed to load texture '" << path << "'.\n";
        return getFallbackTexture();
    }

    textures.emplace(name, tex);
    return textures[name];
}

Texture2D& ResourceManager::GetTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) return it->second;

    std::cerr << "Failed to retrieve texture '" << name << "'.\n";
    return getFallbackTexture();
}

Model& ResourceManager::LoadModel(const std::string& name, const std::string& path) {
    auto it = models.find(name);
    if (it != models.end()) return it->second;
    Model m = ::LoadModel(path.c_str());
    models.emplace(name, m);
    return models[name];
}

Model& ResourceManager::LoadModelFromMesh(const std::string& name, const Mesh& mesh) {
    auto it = models.find(name);
    if (it != models.end()) return it->second;
    Model m = ::LoadModelFromMesh(mesh);
    models.emplace(name, m);
    return models[name];
}
Model& ResourceManager::GetModel(const std::string& name) const {
    auto it = models.find(name);
    if (it == models.end()) throw std::runtime_error("Model not found: " + name);
    return const_cast<Model&>(it->second);
}

Shader& ResourceManager::LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
    auto it = shaders.find(name);
    if (it != shaders.end()) return it->second;
    Shader s = ::LoadShader(vsPath.c_str(), fsPath.c_str());
    shaders.emplace(name, s);
    return shaders[name];
}
Shader& ResourceManager::GetShader(const std::string& name) const {
    auto it = shaders.find(name);
    if (it == shaders.end()) throw std::runtime_error("Shader not found: " + name);
    return const_cast<Shader&>(it->second);
}

RenderTexture2D& ResourceManager::LoadRenderTexture(const std::string& name, int w, int h) { 
    auto it = renderTextures.find(name); if (it != renderTextures.end()) return it->second; 
    RenderTexture2D rt = ::LoadRenderTexture(w, h); renderTextures.emplace(name, rt); 
    return renderTextures[name]; 
}

RenderTexture2D& ResourceManager::GetRenderTexture(const std::string& name) const {
    auto it = renderTextures.find(name);
    if (it == renderTextures.end()) throw std::runtime_error("RenderTexture not found: " + name);
    return const_cast<RenderTexture2D&>(it->second);
}

Font& ResourceManager::LoadFont(const std::string& name,const std::string& path) {
    auto it = fonts.find(name);
    if (it != fonts.end()) return it->second;

    Font f = ::LoadFontEx(path.c_str(), 120, nullptr, 0);
    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
    if (f.texture.id == 0) {
        std::cerr << "Failed to load font '" << path << "'.\n";
        return getFallbackFont();
    }

    fonts.emplace(name, f);
    return fonts[name];
}

Font& ResourceManager::GetFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it != fonts.end()) return it->second;
    
    std::cerr << "Failed to retrieve font '" << name << "'.\n";
    return getFallbackFont();
}

void ResourceManager::LoadAllResources() {
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };

    auto& scene = LoadRenderTexture("sceneTexture", screenResolution.x, screenResolution.y);
    SetTextureFilter(scene.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(scene.texture, TEXTURE_WRAP_CLAMP);

    auto& post = LoadRenderTexture("postProcessTexture", screenResolution.x, screenResolution.y);
    SetTextureFilter(post.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(post.texture, TEXTURE_WRAP_CLAMP);

    SetTextureWrap(GetRenderTexture("sceneTexture").texture, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(GetRenderTexture("postProcessTexture").texture, TEXTURE_WRAP_CLAMP);

    for (const auto& file : std::filesystem::directory_iterator("assets/fonts/")) {
        LoadFont(file.path().stem().string(), file.path().string());
    }

    for (const auto& file : std::filesystem::directory_iterator("assets/sprites/")) {
        LoadTexture(file.path().stem().string(), file.path().string());
    }
    LoadTexture("backDrop", "assets/screenshots/dungeon1.png");

    for (const auto& file : std::filesystem::directory_iterator("assets/Models/")) {
        LoadModel(file.path().stem().string(), file.path().string());
    }

    //generated models
    LoadModelFromMesh("skyModel", GenMeshCube(1.0f, 1.0f, 1.0f));
    LoadModelFromMesh("waterModel",GenMeshPlane(30000, 30000, 1, 1));
    LoadModelFromMesh("bottomPlane",GenMeshPlane(30000, 30000, 1, 1));
    LoadModelFromMesh("shadowQuad",GenMeshPlane(1.0f, 1.0f, 1, 1));

    //shaders, TODO: replace with an automatic loading system, too lazy right now
    LoadShader("terrainShader", "assets/shaders/height_color.vs",      "assets/shaders/height_color.fs");
    LoadShader("fogShader",     /*vsPath=*/"",                         "assets/shaders/fog_postprocess.fs");
    LoadShader("shadowShader",  "assets/shaders/shadow_decal.vs",      "assets/shaders/shadow_decal.fs");
    LoadShader("skyShader",     "assets/shaders/skybox.vs",            "assets/shaders/skybox.fs");
    LoadShader("waterShader",   "assets/shaders/water.vs",             "assets/shaders/water.fs");
    LoadShader("bloomShader",   /*vsPath=*/"",                         "assets/shaders/bloom.fs");
    LoadShader("cutoutShader",                        "",              "assets/shaders/leaf_cutout.fs");
    LoadShader("lightingShader","assets/shaders/lighting_baked_xz.vs", "assets/shaders/lighting_baked_xz.fs");
    LoadShader("lavaShader",    "assets/shaders/lava_world.vs",        "assets/shaders/lava_world.fs");
    LoadShader("treeShader", "assets/shaders/treeShader.vs",           "assets/shaders/treeShader.fs");
    LoadShader("portalShader", "assets/shaders/portal.vs",             "assets/shaders/portal.fs");
}

// Unload functions

void ResourceManager::UnloadAll() {
    for (auto& [_, texture] : textures) {
        UnloadTexture(texture);
    }

    for (auto& [_, model] : models) {
        UnloadModel(model);
    }

    for (auto& [_, shader] : shaders) {
        UnloadShader(shader);
    }

    for (auto& [_, renderTexture] : renderTextures) {
        UnloadRenderTexture(renderTexture);
    }

    // Do not unload default Raylib font. Fallback texture does not get unloaded for simplicity and
    // it is not important as unloading happens at the termination of the program.
    for (auto& [_, font] : fonts) {
        if (font.texture.id != GetFontDefault().texture.id) {
            UnloadFont(font);
        }
    }
}

// Shader functions

void ResourceManager::UpdateShaders(Camera& camera){
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };

    Shader& waterShader = GetShader("waterShader");
    Shader& skyShader = GetShader("skyShader");
    Shader& terrainShader = GetShader("terrainShader");
    Shader& fogShader = GetShader("fogShader");
    Shader& bloomShader = GetShader("bloomShader");
    Shader& treeShader = GetShader("treeShader");

    Vector3 camPos = camera.position;

    float t = GetTime();
    int dungeonFlag = isDungeon;
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

    //dungeonDarkness //is there a reason we need to set these every frame? 
    float dungeonDarkness = -0.1f;//it darkens the gun model as well, so go easy. negative number brightens it. 
    float dungeonContrast = 1.00f; //makes darks darker. 

    int isDungeonVal = isDungeon ? 1 : 0;
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "isDungeon"), &isDungeonVal, SHADER_UNIFORM_INT);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "dungeonDarkness"), &dungeonDarkness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(fogShader, GetShaderLocation(fogShader, "dungeonContrast"), &dungeonContrast, SHADER_UNIFORM_FLOAT);
    if (!isLoadingLevel){
        Shader lavaShader = GetShader("lavaShader");
        int locTime        = GetShaderLocation(lavaShader, "uTime");
        SetShaderValue(GetShader("lavaShader"), locTime, &t, SHADER_UNIFORM_FLOAT);

    }
    //tree shadows
    // Once (cache locations)
    int locShadow      = GetShaderLocation(terrainShader, "u_ShadowMask");
    int locWorldMinXZ  = GetShaderLocation(terrainShader, "u_WorldMinXZ");
    int locWorldSizeXZ = GetShaderLocation(terrainShader, "u_WorldSizeXZ");

    // Per-frame before drawing terrain:
    Vector2 worldMinXZ  = { gTreeShadowMask.worldXZBounds.x, gTreeShadowMask.worldXZBounds.y };
    Vector2 worldSizeXZ = { gTreeShadowMask.worldXZBounds.width, gTreeShadowMask.worldXZBounds.height };
    //std::cout << gTreeShadowMask.rt.texture.id << " id\n" << gTreeShadowMask.rt.texture.width << " width\n";

    SetShaderValue(terrainShader, locWorldMinXZ,  &worldMinXZ,  SHADER_UNIFORM_VEC2);
    SetShaderValue(terrainShader, locWorldSizeXZ, &worldSizeXZ, SHADER_UNIFORM_VEC2);
    SetShaderValueTexture(terrainShader, locShadow, gTreeShadowMask.rt.texture);

    //distance fog
    int locCam_Terrain = GetShaderLocation(terrainShader, "cameraPos");
    int locCam_Trees   = GetShaderLocation(treeShader,   "cameraPos");

    SetShaderValue(terrainShader, locCam_Terrain, &camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(treeShader,   locCam_Trees,   &camPos, SHADER_UNIFORM_VEC3);

    int loc_time_p = GetShaderLocation(GetShader("portalShader"), "u_time");
    //portal
    SetShaderValue(GetShader("portalShader"), loc_time_p, &t, SHADER_UNIFORM_FLOAT);
}

void ResourceManager::SetBloomShaderValues(){
    //bloom post process. 
    Shader& bloomShader = GetShader("bloomShader");
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };
    bloomStrengthValue = 0.0f;
    float bloomColor[3] = { 1.0f, 1.0f, 1.0f };  
    float aaStrengthValue = 0.0f; //blur

    float bloomThreshold = 0.1f;  // e.g. 1.0 in sRGB ≈ ~0.8 linear; start around 0.7–1.2
    float bloomKnee = 0.0; 

    //tonemap
    float exposure = isDungeon ? 1.0 : 0.9; // need to trigger on level switch
    int toneOp = isDungeon;

    float lavaBoot = 5.0f;

    int locLB = GetShaderLocation(bloomShader, "lavaBoost");
    SetShaderValue(bloomShader, locLB, &lavaBoot, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "uExposure"), &exposure, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "uToneMapOperator"), &toneOp, SHADER_UNIFORM_INT);

    vignetteStrengthValue = 0.35f; //darker vignette in dungeons
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "resolution"), &screenResolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomColor"), bloomColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "aaStrength"), &aaStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomThreshold"), &bloomThreshold, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "bloomKnee"), &bloomKnee, SHADER_UNIFORM_FLOAT);
}

void ResourceManager::SetLavaShaderValues(){
    Shader lavaShader = GetShader("lavaShader");
    Model& lavaTile = GetModel("lavaTile");

    for (int i=0; i < lavaTile.materialCount; i++){
        lavaTile.materials[i].shader = lavaShader;
    }

    // Hook locations once
    int locTime   = GetShaderLocation(lavaShader, "uTime");
    int locDir    = GetShaderLocation(lavaShader, "uScrollDir");
    int locSpeed  = GetShaderLocation(lavaShader, "uSpeed");
    int locOff    = GetShaderLocation(lavaShader, "uWorldOffset");
    int locScale  = GetShaderLocation(lavaShader, "uUVScale");
    int locFreq   = GetShaderLocation(lavaShader, "uDistortFreq");
    int locAmp    = GetShaderLocation(lavaShader, "uDistortAmp");
    int locEmis   = GetShaderLocation(lavaShader, "uEmissive");
    int locGain   = GetShaderLocation(lavaShader, "uEmissiveGain");

    // Parameters
    Vector2 scroll = { 0.07f, 0.0f };
    float speed = 0.5f, freq = 6.0f, amp = 0.02f;
    float gain = 0.1f;
    Vector3 emis = { 3.0f, 1.0f, 0.08f };

    // World-to-UV scale:
    //   If your tile is 'tileSize' world units and you want 1 repeat per *tile*,
    //   then uUVScale = 1.0f / tileSize.
    //   If you want 2 repeats per tile: 2.0f / tileSize, etc.
    float uvsPerWorldUnit = 0.5 / tileSize;

    // Dungeon/world origin 
    Vector2 worldOffset = {0,0};

    // Set static params once
    SetShaderValue(lavaShader, locDir,   &scroll,       SHADER_UNIFORM_VEC2);
    SetShaderValue(lavaShader, locSpeed, &speed,        SHADER_UNIFORM_FLOAT);
    SetShaderValue(lavaShader, locFreq,  &freq,         SHADER_UNIFORM_FLOAT);
    SetShaderValue(lavaShader, locAmp,   &amp,          SHADER_UNIFORM_FLOAT);
    SetShaderValue(lavaShader, locEmis,  &emis,         SHADER_UNIFORM_VEC3);
    SetShaderValue(lavaShader, locGain,  &gain,         SHADER_UNIFORM_FLOAT);
    SetShaderValue(lavaShader, locOff,   &worldOffset,  SHADER_UNIFORM_VEC2);
    SetShaderValue(lavaShader, locScale, &uvsPerWorldUnit, SHADER_UNIFORM_FLOAT);
}

void ResourceManager::SetLightingShaderValues() {
    Shader lightingShader = GetShader("lightingShader");

    Model& floorModel = GetModel("floorTileGray");
    Model& wallModel = GetModel("wallSegment");
    Model& doorwayModel = GetModel("doorWayGray");
    Model& launcherModel = GetModel("stonePillar");
    Model& barrelModel = GetModel("barrelModel");
    Model& brokeModel = GetModel("brokeBarrel");
    // Bind shader to models
    for (int i = 0; i < wallModel.materialCount; i++)    wallModel.materials[i].shader = lightingShader;
    for (int i = 0; i < doorwayModel.materialCount; i++) doorwayModel.materials[i].shader = lightingShader;
    for (int i = 0; i < floorModel.materialCount; ++i)   floorModel.materials[i].shader = lightingShader;
    for (int i = 0; i < launcherModel.materialCount; ++i)   launcherModel.materials[i].shader = lightingShader;
    for (int i = 0; i < barrelModel.materialCount; ++i)   barrelModel.materials[i].shader = lightingShader;
    for (int i = 0; i < brokeModel.materialCount; ++i)   brokeModel.materials[i].shader = lightingShader;

    // Use one material's shader handle to set uniforms (shared Shader)
    Shader use = floorModel.materials[0].shader;

    // Existing uniforms
    int locGrid   = GetShaderLocation(use, "gridBounds");
    int locDynTex = GetShaderLocation(use, "dynamicGridTex");
    int locDynStr = GetShaderLocation(use, "dynStrength");
    int locAmb    = GetShaderLocation(use, "ambientBoost");

    float grid[4] = {
        gDynamic.minX, gDynamic.minZ,
        gDynamic.sizeX ? 1.0f / gDynamic.sizeX : 0.0f,
        gDynamic.sizeZ ? 1.0f / gDynamic.sizeZ : 0.0f
    };
    if (locGrid   >= 0) SetShaderValue(use, locGrid, grid, SHADER_UNIFORM_VEC4);

    float dynStrength  = 0.8f;
    float ambientBoost = 0.2f;

    if (!isDungeon) { // entrances fully lit
        dynStrength  = 0.0f;
        ambientBoost = 1.0f;
    }

    if (locDynStr >= 0) SetShaderValue(use, locDynStr, &dynStrength,  SHADER_UNIFORM_FLOAT);
    if (locAmb    >= 0) SetShaderValue(use, locAmb,    &ambientBoost, SHADER_UNIFORM_FLOAT);

    if (locDynTex >= 0) SetShaderValueTexture(use, locDynTex, gDynamic.tex);

    // --- New lava/ceiling uniforms ---
    int locIsCeil   = GetShaderLocation(use, "isCeiling");        // int
    int locLavaStr  = GetShaderLocation(use, "lavaCeilStrength"); // float
    int locCeilH    = GetShaderLocation(use, "ceilHeight");       // float
    int locLavaFall = GetShaderLocation(use, "lavaFalloff");      // float

    // Sensible defaults (you can tweak live)
    int   isCeilDefault   = 0;                 // floors by default
    float lavaCeilStrength= 0.15f;             // try 0.4–0.7
    float ceilH           = ceilingHeight;     // your world Y for ceilings
    float lavaFalloff     = 600.0f;            // how fast ceiling glow fades with height

    if (locIsCeil   >= 0) SetShaderValue(use, locIsCeil,   &isCeilDefault,    SHADER_UNIFORM_INT);
    if (locLavaStr  >= 0) SetShaderValue(use, locLavaStr,  &lavaCeilStrength, SHADER_UNIFORM_FLOAT);
    if (locCeilH    >= 0) SetShaderValue(use, locCeilH,    &ceilH,            SHADER_UNIFORM_FLOAT);
    if (locLavaFall >= 0) SetShaderValue(use, locLavaFall, &lavaFalloff,      SHADER_UNIFORM_FLOAT);
}

void ResourceManager::SetTerrainShaderValues(){ //plus palm tree shader
    // Load textures (tileable, power-of-two helps mips)
    Shader& terrainShader = GetShader("terrainShader");
    Texture2D grassTex = GetTexture("grassTexture");
    Texture2D sandTex = GetTexture("sandTexture");

    // Make them look good when tiled
    GenTextureMipmaps(&grassTex);
    GenTextureMipmaps(&sandTex);
    SetTextureFilter(grassTex, TEXTURE_FILTER_TRILINEAR);
    SetTextureFilter(sandTex,  TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(grassTex, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(sandTex,  TEXTURE_WRAP_REPEAT);

    // Hook shader samplers to material map locations
    terrainShader.locs[SHADER_LOC_MAP_ALBEDO]     = GetShaderLocation(terrainShader, "texGrass");
    terrainShader.locs[SHADER_LOC_MAP_METALNESS]  = GetShaderLocation(terrainShader, "texSand");
    terrainShader.locs[SHADER_LOC_MAP_OCCLUSION]  = GetShaderLocation(terrainShader, "textureOcclusion");

    // Assign shader and maps to the terrain material
    terrainModel.materials[0].shader = terrainShader;
    SetMaterialTexture(&terrainModel.materials[0], MATERIAL_MAP_ALBEDO,    grassTex);
    SetMaterialTexture(&terrainModel.materials[0], MATERIAL_MAP_METALNESS, sandTex);

    //terrain
    // Uniforms for world bounds & tiling
    int locWorldMinXZ  = GetShaderLocation(terrainShader, "u_WorldMinXZ");
    int locWorldSizeXZ = GetShaderLocation(terrainShader, "u_WorldSizeXZ");
    int locGrassTile   = GetShaderLocation(terrainShader, "grassTiling");
    int locSandTile    = GetShaderLocation(terrainShader, "sandTiling");
    
    Vector2 t_worldMinXZ  = { -terrainScale.x*0.5f, -terrainScale.z*0.5f };
    Vector2 t_worldSizeXZ = {  terrainScale.x,       terrainScale.z      };
    SetShaderValue(terrainShader, locWorldMinXZ,  &t_worldMinXZ,  SHADER_UNIFORM_VEC2);
    SetShaderValue(terrainShader, locWorldSizeXZ, &t_worldSizeXZ, SHADER_UNIFORM_VEC2);

    // Tiling counts (start here; tweak live)
    float grassTiles = 60.0f;   // repeats across island width
    float sandTiles  = 20.0f;
    SetShaderValue(terrainShader, locGrassTile, &grassTiles, SHADER_UNIFORM_FLOAT);
    SetShaderValue(terrainShader, locSandTile,  &sandTiles,  SHADER_UNIFORM_FLOAT);

    //distant fog
    int locSkyTop   = GetShaderLocation(terrainShader, "u_SkyColorTop");
    int locSkyHorz  = GetShaderLocation(terrainShader, "u_SkyColorHorizon");
    int locFogStart = GetShaderLocation(terrainShader, "u_FogStart");
    int locFogEnd   = GetShaderLocation(terrainShader, "u_FogEnd");
    int locSea      = GetShaderLocation(terrainShader, "u_SeaLevel");
    int locFalloff  = GetShaderLocation(terrainShader, "u_FogHeightFalloff");
    //0.0, 0.60, 1.00
    // nice tropical-ish defaults
    Vector3 skyTop  = {0.55f, 0.75f, 1.00f};
    Vector3 skyHorz = {0.6f, 0.8f, 0.95f};
    float fogStart  = 100.0f;
    float fogEnd    = 18000.0f;
    float seaLevel  = 400.0f;     // your existing cutoff
    float falloff   = 0.002f;    // smaller = thinner with height

    SetShaderValue(terrainShader, locSkyTop,   &skyTop,  SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, locSkyHorz,  &skyHorz, SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, locFogStart, &fogStart,SHADER_UNIFORM_FLOAT);
    SetShaderValue(terrainShader, locFogEnd,   &fogEnd,  SHADER_UNIFORM_FLOAT);
    SetShaderValue(terrainShader, locSea,      &seaLevel,SHADER_UNIFORM_FLOAT);
    SetShaderValue(terrainShader, locFalloff,  &falloff, SHADER_UNIFORM_FLOAT);

    // Load tree shader
    Shader treeShader = GetShader("treeShader");
    Model& treeModel = GetModel("palmTree");
    Model& smallTreeModel = GetModel("palm2");
    Model& bushModel = GetModel("bush");
    Model& doorwayModel = GetModel("doorWayGray");

    // Hook ALBEDO to our sampler name
    treeShader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(treeShader, "textureDiffuse");

    // (optional) hook diffuse tint if you plan to use it; otherwise raylib will handle it
    treeShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(treeShader, "colDiffuse");

    // Set shared fog uniforms once (reuse the same values as terrain)
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"u_SkyColorTop"),      &skyTop,   SHADER_UNIFORM_VEC3);
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"u_SkyColorHorizon"),  &skyHorz,  SHADER_UNIFORM_VEC3);
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"u_FogStart"),         &fogStart, SHADER_UNIFORM_FLOAT);
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"u_FogEnd"),           &fogEnd,   SHADER_UNIFORM_FLOAT);
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"u_SeaLevel"),         &seaLevel, SHADER_UNIFORM_FLOAT);
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"u_FogHeightFalloff"), &falloff,  SHADER_UNIFORM_FLOAT);

    // Alpha cutoff (tweak per asset)
    float alphaCut = 0.30f;
    SetShaderValue(treeShader, GetShaderLocation(treeShader,"alphaCutoff"), &alphaCut, SHADER_UNIFORM_FLOAT);

    for (int i = 0; i < doorwayModel.materialCount; ++i) {
        doorwayModel.materials[i].shader = treeShader;
        doorwayModel.materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    }

    for (int i = 0; i < bushModel.materialCount; ++i) {
        bushModel.materials[i].shader = treeShader;
        bushModel.materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    }

    for (int i = 0; i < treeModel.materialCount; ++i) {
        treeModel.materials[i].shader = treeShader;
        smallTreeModel.materials[i].shader = treeShader;
        
        // IMPORTANT: keep map tint white so it doesn’t darken
        treeModel.materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        smallTreeModel.materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    }
}

void ResourceManager::SetPortalShaderValues(){
    Shader portal = GetShader("portalShader");

    int loc_speed         = GetShaderLocation(portal, "u_speed");
    int loc_swirlStrength = GetShaderLocation(portal, "u_swirlStrength");
    int loc_swirlScale    = GetShaderLocation(portal, "u_swirlScale");
    int loc_colorA        = GetShaderLocation(portal, "u_colorA");
    int loc_colorB        = GetShaderLocation(portal, "u_colorB");
    int loc_edgeFeather   = GetShaderLocation(portal, "u_edgeFeather");
    int loc_rings         = GetShaderLocation(portal, "u_rings");
    int loc_glowBoost     = GetShaderLocation(portal, "u_glowBoost");

    // One-time defaults
    float loc_speed_ptr[] {1.4f};
    float loc_swirlStrength_ptr[] {1.4f};
    float loc_swirlScale_ptr[] {12.0f};
    float loc_edgeFeather_ptr[] {0.08f};
    float loc_rings_ptr[] {0.7f};
    float loc_glowBoost_ptr[] {0.8f};

    SetShaderValue(portal, loc_speed,         loc_speed_ptr, SHADER_UNIFORM_FLOAT);
    SetShaderValue(portal, loc_swirlStrength, loc_swirlStrength_ptr, SHADER_UNIFORM_FLOAT);
    SetShaderValue(portal, loc_swirlScale,    loc_swirlScale_ptr, SHADER_UNIFORM_FLOAT);
    SetShaderValue(portal, loc_edgeFeather,   loc_edgeFeather_ptr, SHADER_UNIFORM_FLOAT);
    SetShaderValue(portal, loc_rings,         loc_rings_ptr, SHADER_UNIFORM_FLOAT);
    SetShaderValue(portal, loc_glowBoost,     loc_glowBoost_ptr, SHADER_UNIFORM_FLOAT);

    // Colors 
    Vector3 cA = {0.0f, 0.25f, 1.0f};
    Vector3 cB = {0.5f, 0.2f, 1.0f};
    SetShaderValue(portal, loc_colorA, &cA, SHADER_UNIFORM_VEC3);
    SetShaderValue(portal, loc_colorB, &cB, SHADER_UNIFORM_VEC3);
}

void ResourceManager::SetShaderValues(){
    //outdoor shaders + bloom, tonemap, saturation, foliage alpha cutoff , shadowTex
    Vector2 screenResolution = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };
    // set shaders values
    Shader& fogShader = GetShader("fogShader");
    Shader& shadowShader = GetShader("shadowShader");
    Shader& waterShader = GetShader("waterShader");
    Shader& terrainShader = GetShader("terrainShader");

    // Sky
    //apply skyShader to sky model
    GetModel("skyModel").materials[0].shader = GetShader("skyShader");

    // Load & assign once
    Shader sky = GetShader("skyShader");
    GetModel("skyModel").materials[0].shader = sky;

    // Cache uniform locations
    int locTime      = GetShaderLocation(sky, "time");
    int locIsDungeon = GetShaderLocation(sky, "isDungeon");

    int isDung = isDungeon ? 1 : 0; 
    SetShaderValue(sky, locIsDungeon, &isDung, SHADER_UNIFORM_INT);


    // Shadow //Shadow decals beneath trees. Also shadows beneath enemies. 
    Model& shadowQuad = GetModel("shadowQuad");
    shadowQuad.materials[0].shader = shadowShader;
    SetMaterialTexture(&shadowQuad.materials[0], MATERIAL_MAP_DIFFUSE, GetTexture("shadowTex"));

    // Water
    
    SetShaderValue(waterShader, GetShaderLocation(waterShader, "waterLevel"), &waterHeightY, SHADER_UNIFORM_FLOAT);
    GetModel("waterModel").materials[0].shader = waterShader;
    GetModel("bottomPlane").materials[0].shader = waterShader;

    Shader& bloomShader = GetShader("bloomShader");

    //tonemap
    float exposure = isDungeon ? 1.0 : 0.9; // needed for both dungeons and outdoor level
    int toneOp = isDungeon ? 1 : 0;
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "uExposure"), &exposure, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "uToneMapOperator"), &toneOp, SHADER_UNIFORM_INT);

    int locSat = GetShaderLocation(bloomShader, "uSaturation"); //also needed for overworld map
    float sat = 1.0; // try 1.05–1.25
    SetShaderValue(bloomShader, locSat, &sat, SHADER_UNIFORM_FLOAT); 
}

void ResourceManager::EnsureScreenSizedRTs() {
    static int gLastW = -1, gLastH = -1;
    int w = GetScreenWidth(), h = GetScreenHeight();
    if (w == gLastW && h == gLastH) return;

    // Recreate sceneTexture
    if (renderTextures.count("sceneTexture")) {
        UnloadRenderTexture(renderTextures["sceneTexture"]);
    }
    renderTextures["sceneTexture"] = LoadRenderTexture("sceneTexture", w, h);
    SetTextureWrap(renderTextures["sceneTexture"].texture, TEXTURE_WRAP_CLAMP);

    // Recreate postProcessTexture
    if (renderTextures.count("postProcessTexture")) {
        UnloadRenderTexture(renderTextures["postProcessTexture"]);
    }
    renderTextures["postProcessTexture"] = LoadRenderTexture("postProcessTexture", w, h);
    SetTextureWrap(renderTextures["postProcessTexture"].texture, TEXTURE_WRAP_CLAMP);

    gLastW = w; gLastH = h;
}

// Fallback functions

Texture2D& ResourceManager::getFallbackTexture() {
    static Texture2D texture;
    static bool loaded = false;

    if (not loaded) {
        Image img = GenImageChecked(64, 64, 8, 8, MAGENTA, BLACK);
        texture = LoadTextureFromImage(img);
        UnloadImage(img);
        loaded = true; // This will only fail in extreme situations, who cares
    }
    return texture;
}

Font& ResourceManager::getFallbackFont() {
    static Font font = GetFontDefault();
    return font;
}
