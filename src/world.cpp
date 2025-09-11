#include "world.h"
#include "player.h"
#include "raymath.h"
#include "resourceManager.h"
#include "vegetation.h"
#include "dungeonGeneration.h"
#include "boat.h"
#include "algorithm"
#include "sound_manager.h"
#include <iostream>
#include "pathfinding.h"
#include "camera_system.h"
#include "list"
#include "assert.h"
#include "lighting.h"



GameState currentGameState = GameState::Menu;

//global variables, clean these up somehow. 
Model terrainModel;
Image heightmap;
Mesh terrainMesh;
Vector3 terrainScale = {16000.0f, 200.0f, 16000.0f}; //very large x and z, 

int levelIndex = 0; //current level, levels[levelIndex]
int previousLevelIndex = 0;
bool first = true; //for first player start position
bool controlPlayer = false;
bool isDungeon = false;
unsigned char* heightmapPixels = nullptr;
Player player = {};
Vector3 startPosition = {5475.0f, 300.0f, -5665.0f}; //middle island start pos
Vector3 boatPosition = {6000, -20, 0.0};
Vector3 playerSpawnPoint = {0,0,0};
Vector3 waterPos = {0, 0, 0};
int pendingLevelIndex = -1; //wait to switch level until faded out. UpdateFade() needs to know the next level index. 

float waterHeightY = 60;
Vector3 bottomPos = {0, waterHeightY - 100, 0};
float dungeonPlayerHeight = 100;
float ceilingHeight = 480;
float fadeToBlack = 0.0f;
float vignetteIntensity = 0.0f;
float vignetteFade = 0.0f;
float vignetteStrengthValue = 0.2;
float bloomStrengthValue = 0.0;
bool isFading = false;
float fadeSpeed = 1.0f; // units per second
bool fadeIn = true; 
float tileSize = 200;
bool switchFromMenu = false;
int selectedOption = 0;
float floorHeight = 80;
float wallHeight = 270;
float dungeonEnemyHeight = 165;
float ElapsedTime = 0.0f;
bool debugInfo = false;
bool isLoadingLevel = false;
float weaponDarkness = 0.0f;

//std::vector<Bullet> activeBullets;
std::list<Bullet> activeBullets; // instead of std::vector

std::vector<Decal> decals;
std::vector<MuzzleFlash> activeMuzzleFlashes;
std::vector<Collectable> collectables;
std::vector<CollectableWeapon> worldWeapons; //weapon pickups
std::vector<Character> enemies;  
std::vector<Character*> enemyPtrs;
std::vector<DungeonEntrance> dungeonEntrances;


void InitLevel(const LevelData& level, Camera& camera) {
    isLoadingLevel = true;

    //Called when starting game and changing level. init the level you pass it. the level is chosen by menu or door's linkedLevelIndex. 
    ClearLevel();//clears everything. 

    camera.position = player.position; //start as player, not freecam.
    levelIndex = level.levelIndex; //update current level index to new level. 

    vignetteStrengthValue = 0.2f; //less of vignette outdoors.
    bloomStrengthValue = 0.0f; //turn on bloom in dungeons
    SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);
    
    // Load and format the heightmap image
    heightmap = LoadImage(level.heightmapPath.c_str());
    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    
    terrainMesh = GenMeshHeightmap(heightmap, terrainScale);
    terrainModel = LoadModelFromMesh(terrainMesh);
    terrainModel.materials[0].shader = R.GetShader("terrainShader");
    dungeonEntrances = level.entrances; //get level entrances from level data

    generateRaptors(level.raptorCount, level.raptorSpawnCenter, 6000);
    GenerateEntrances();
    generateVegetation(); 
    if (!level.isDungeon) InitBoat(player_boat, boatPosition);
   
    if (level.isDungeon){
        isDungeon = true;
        vignetteStrengthValue = 0.5f; //darker vignette in dungeons
        bloomStrengthValue = 0.15f; //turn on bloom in dungeons
        SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "vignetteStrength"), &vignetteStrengthValue, SHADER_UNIFORM_FLOAT);
        SetShaderValue(R.GetShader("bloomShader"), GetShaderLocation(R.GetShader("bloomShader"), "bloomStrength"), &bloomStrengthValue, SHADER_UNIFORM_FLOAT);

        LoadDungeonLayout(level.dungeonPath);
        ConvertImageToWalkableGrid(dungeonImg);
        GenerateLightSources(floorHeight);
        GenerateFloorTiles(floorHeight);//80
        GenerateWallTiles(wallHeight); //model is 400 tall with origin at it's center, so wallHeight is floorHeight + model height/2. 270
        GenerateCeilingTiles(ceilingHeight);//400
        GenerateBarrels(floorHeight);
        GenerateLaunchers(floorHeight);
        GenerateSpiderWebs(floorHeight);
        GenerateChests(floorHeight);
        GeneratePotions(floorHeight);
        GenerateKeys(floorHeight);
        GenerateWeapons(200);
        
        GenerateDoorways(floorHeight, levelIndex); //calls generate doors from archways
        //generate enemies.
        GenerateSkeletonsFromImage(dungeonEnemyHeight); //165
        GeneratePiratesFromImage(dungeonEnemyHeight);
        GenerateSpiderFromImage(dungeonEnemyHeight);
        GenerateGhostsFromImage(dungeonEnemyHeight);

        if (levelIndex == 4) levels[0].startPosition = {-5653, 200, 6073}; //exit dungeon 3 to dungeon enterance 2 position.


 
    }



    //XZ dynamic lightmap + shader lighting with occlusion
    InitDungeonLights();
    isLoadingLevel = false;



    Vector3 resolvedSpawn = ResolveSpawnPoint(level, isDungeon, first, floorHeight);
    InitPlayer(player, resolvedSpawn); //start at green pixel if there is one. otherwise level.startPos or first startPos
    CameraSystem::Get().SnapAllToPlayer(); //put freecam at player pos

    //start with blunderbus and sword in that order
    player.collectedWeapons = {WeaponType::Blunderbuss, WeaponType::Sword};
    player.activeWeapon = WeaponType::Blunderbuss;
    player.currentWeaponIndex = 0;


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
                    currentGameState = GameState::Menu; //HACK //quickly switch to menu before switching to new level. This fixes lighting bug on level switch.
                    //Menu gameState stops all other code from running, letting us switch lightmaps cleanly, found no other way. 
                    switchFromMenu = true;
                    //InitLevel(levels[pendingLevelIndex], camera); //Start new Level
                    //pendingLevelIndex = -1;

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
        Shader& fogShader = R.GetShader("fogShader");
        SetShaderValue(fogShader, GetShaderLocation(fogShader, "fadeToBlack"), &fadeToBlack, SHADER_UNIFORM_FLOAT);
    }


}

void InitDungeonLights(){
    InitDynamicLightmap(dungeonWidth * 4); //128 for 32 pixel map. keep same ratio if bigger map. 

    ResourceManager::Get().SetLightingShaderValues();
    
    BuildStaticLightmapOnce(dungeonLights);
    BuildDynamicLightmapFromFrameLights(frameLights); // build dynamic light map once for good luck.

    //TraceLog(LOG_INFO, "dynTex.id=%d glowTex.id=%d", gDynamic.tex.id, gLavaGlow.tex.id);
}

void HandleWaves(){
    //water
    float wave = sin(GetTime() * 0.9f) * 0.9f;  // slow, subtle vertical motion
    float animatedWaterLevel = waterHeightY + wave;
    waterPos = {0, animatedWaterLevel, 0};
    bottomPos = {0, waterHeightY - 100, 0};
}


void removeAllCharacters(){
    enemies.clear();
    enemyPtrs.clear();

}

// Darkness factor should be in [0.0, 1.0]
// 0.0 = fully dark, 1.0 = fully lit
float CalculateDarknessFactor(Vector3 playerPos, const std::vector<LightSource>& lights) {
    float maxLight = 0.0f;
    const float lightRange = 700.0f;


    for (const LightSource& l : lights) {
        float dist = Vector3Distance(playerPos, l.position);
        float contribution = Clamp(1.0f - dist / l.range, 0.0f, 1.0f) * l.intensity;
        maxLight = fmax(maxLight, contribution);
    }

    for (const LightSample& ls : frameLights){
        float dist = Vector3Distance(playerPos, ls.pos);
        float contribution = Clamp(1.0f - dist / lightRange, 0.0f, 1.0);
        maxLight = fmax(maxLight, contribution);
        
    }

    // Invert to get "darkness"
    float darkness = 1.0f - Clamp(maxLight, 0.0f, 1.0f);
    return darkness;
}



void HandleWeaponTints(){
    weaponDarkness = CalculateDarknessFactor(player.position, dungeonLights);

}

void GenerateEntrances(){
    for (const DungeonEntrance& e : dungeonEntrances) {
        Door d{};
        d.position = e.position;
        d.rotationY = 0.0f; 
        d.doorTexture = R.GetTexture("doorTexture");
        d.isOpen = false;
        d.isLocked = false;
        d.scale = {300, 365, 1};
        d.tint = WHITE;
        d.linkedLevelIndex = e.linkedLevelIndex; //use entrance's linkedLevelIndex to determine which dungeon to enter from overworld
        //allowing more than one enterance to dungeon per overworld

        d.doorType = DoorType::GoToNext; //Go to Dungeon

        float halfWidth = 200.0f;   // Half of the 400-unit wide doorway
        float height = 365.0f;
        float depth = 20.0f;        // Thickness into the doorway (forward axis)

        d.collider = MakeDoorBoundingBox(d.position, d.rotationY, halfWidth, height, depth); //the whole archway is covered by collider

        doors.push_back(d);

        DoorwayInstance dw{};
        dw.position = e.position;
        dw.rotationY = 90.0f * DEG2RAD; //rotate to match door 0 rotation, we could rotate door to match arch instead.
        dw.isOpen = false;
        dw.isLocked = false;
        dw.tint = WHITE;

        doorways.push_back(dw);
    }

}

void generateRaptors(int amount, Vector3 centerPos, float radius) {

    int spawned = 0;
    int attempts = 0;
    const int maxAttempts = 1000;
    //try 1000 times to spawn all the raptors either above 80 on heightmap or on empty floor tiles in dungeon. 
    while (spawned < amount && attempts < maxAttempts) {
        ++attempts;

        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float distance = GetRandomValue(500, (int)radius);
        float x = centerPos.x + cosf(angle) * distance;
        float z = centerPos.z + sinf(angle) * distance;

        Vector3 spawnPos = { x, 0.0f, z }; //random x, z  get height diferrently for dungeon
        
        if (isDungeon) {
            // Convert world x,z to dungeon tile coordinates
            const float tileSize = 200.0f; 
            int gridX = (int)(x / tileSize);
            int gridY = (int)(z / tileSize);

            if (!IsDungeonFloorTile(gridX, gridY)) continue; //try again

            float dh = 85.0f;
            float spriteHeight = 200 * 0.5f;
            spawnPos.y = dh + spriteHeight / 2.0f;

        } else {
            float terrainHeight = GetHeightAtWorldPosition(spawnPos, heightmap, terrainScale);
            if (terrainHeight <= 80.0f) continue; //try again

            float spriteHeight = 200 * 0.5f;
            spawnPos.y = terrainHeight + spriteHeight / 2.0f;
        }

        //std::cout << "generated raptor\n";

        Character raptor(spawnPos, R.GetTexture("raptorTexture"), 200, 200, 1, 0.5f, 0.5f, 0, CharacterType::Raptor);
        enemies.push_back(raptor);
        enemyPtrs.push_back(&enemies.back()); 
        ++spawned;
    }


}

void UpdateEnemies(float deltaTime) {
    if (isLoadingLevel) return;
    for (Character& e : enemies){
        e.Update(deltaTime, player);
    }
}

void UpdateMuzzleFlashes(float deltaTime) {
    for (auto& flash : activeMuzzleFlashes) {
        flash.age += deltaTime;
    }

    // Remove expired flashes
    activeMuzzleFlashes.erase(
        std::remove_if(activeMuzzleFlashes.begin(), activeMuzzleFlashes.end(),
                       [](const MuzzleFlash& flash) { return flash.age >= flash.lifetime; }),
        activeMuzzleFlashes.end());
}

void UpdateBullets(Camera& camera, float dt) {
    for (Bullet& b : activeBullets) {
        if (b.IsAlive()) {
            b.Update(camera, dt);
            // (optional) animate b.light.intensity/b.light.range while flying
        } else {
            // First frame of death: convert to glow if requested
            if (b.light.active && b.light.detachOnDeath && !b.light.detached) {
                b.light.detached = true;
                b.light.age = 0.f;
                b.light.posWhenDetached = b.GetPosition();  // freeze at death position
            }
            if (b.light.detached) {
                b.light.age += dt;
                float t = 1.0f - (b.light.age / b.light.lifeTime);
                if (t <= 0.f) {
                    b.light.active = false;      // glow ended
                } else {
                    // optional: b.light.intensity = baseIntensity * t;
                }
            }
        }
    }
    // Don’t erase bullets here—do it after lighting if you need to.
}


void EraseBullets(){
    activeBullets.erase( //erase dead bullets. 
        std::remove_if(activeBullets.begin(), activeBullets.end(),
                    [](const Bullet& b) { return !b.IsAlive(); }),
        activeBullets.end());
}

void UpdateCollectables(float deltaTime) { 
    for (size_t i = 0; i < collectables.size(); i++) {
        collectables[i].Update(deltaTime);

        // Pickup logic
        if (collectables[i].CheckPickup(player.position, 180.0f)) { //180 radius
            if (collectables[i].type == CollectableType::HealthPotion) {
                player.inventory.AddItem("HealthPotion");
                SoundManager::GetInstance().Play("clink");
            }
            else if (collectables[i].type == CollectableType::Key) {
                player.inventory.AddItem("GoldKey");
                SoundManager::GetInstance().Play("key");
            }
            else if (collectables[i].type == CollectableType::Gold) {
                player.gold += collectables[i].value;
                SoundManager::GetInstance().Play("key");
            } else if (collectables[i].type == CollectableType::ManaPotion) {
                player.inventory.AddItem("ManaPotion");
                SoundManager::GetInstance().Play("clink");
            }

            collectables.erase(collectables.begin() + i);
            i--;
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


void DrawBloodParticles(Camera& camera){
    for (Character* enemy : enemyPtrs) { //draw enemy blood, blood is 3d so draw before billboards. 
            enemy->bloodEmitter.Draw(camera);
    }
}

void DrawBullets(Camera& camera) {
    for (const Bullet& b : activeBullets) {
        if (b.IsAlive()){
             b.Draw(camera);
        }
    }

}

void DrawOverworldProps() {
    for (const auto& p : levels[levelIndex].overworldProps) {
        
        const char* modelKey =
            (p.type == PropType::Barrel) ? "barrelModel" :
            (p.type == PropType::FirePit)? "campFire": "barrelModel";

        //std::cout << modelKey << "\n";
        Vector3 propPos = {p.x, 300, p.z};
        float propY = GetHeightAtWorldPosition(propPos, heightmap, terrainScale);
        propPos.y = propY;
        DrawModelEx(R.GetModel(modelKey), propPos,
                    {0,1,0}, p.yawDeg, {p.scale,p.scale,p.scale}, WHITE);
    }
}



Vector3 ResolveSpawnPoint(const LevelData& level, bool isDungeon, bool first, float floorHeight) 
{
    Vector3 resolvedSpawn = level.startPosition; // default fallback

    if (first) {
        resolvedSpawn = {5475.0f, 300.0f, -5665.0f}; // hardcoded spawn for first level
    }

    if (isDungeon) {
        Vector3 pixelSpawn = FindSpawnPoint(dungeonPixels, dungeonWidth, dungeonHeight, tileSize, floorHeight);
        if (pixelSpawn.x != 0 || pixelSpawn.z != 0) {
            resolvedSpawn = pixelSpawn; // green pixel overrides everything
        }
    }

    return resolvedSpawn;
}


float GetHeightAtWorldPosition(Vector3 position, Image& heightmap, Vector3 terrainScale) {
    //read heightmap pixels and return the height in world space. 
    int width = heightmap.width;
    int height = heightmap.height;
    unsigned char* pixels = (unsigned char*)heightmap.data;

    // Convert world X/Z into heightmap image coordinates
    float xPercent = (position.x + terrainScale.x / 2.0f) / terrainScale.x;
    float zPercent = (position.z + terrainScale.z / 2.0f) / terrainScale.z;

    // Clamp to valid range
    xPercent = Clamp(xPercent, 0.0f, 1.0f);
    zPercent = Clamp(zPercent, 0.0f, 1.0f);

    // Convert to pixel indices
    int x = (int)(xPercent * (width - 1));
    int z = (int)(zPercent * (height - 1));
    int index = z * width + x;

    // Get grayscale pixel and scale to world height
    float heightValue = (float)pixels[index] / 255.0f;
    return heightValue * terrainScale.y;
}

void UpdateWorldFrame(float dt, Player& player) {
    // Toggle mode
    if (IsKeyPressed(KEY_TAB) && debugInfo) {
        auto m = CameraSystem::Get().GetMode();
        CameraSystem::Get().SetMode(m == CamMode::Player ? CamMode::Free : CamMode::Player);
    }

    PlayerView pv{
        player.position,
        player.rotation.y,
        player.rotation.x,
        player.isSwimming,
        player.onBoard,
        player_boat.position
    };
    CameraSystem::Get().SyncFromPlayer(pv); //synce to players rotation

    // Update camera (handles free vs player internally)
    CameraSystem::Get().Update(dt);

}

void ClearLevel() {
    billboardRequests.clear();
    removeAllCharacters();\
    activeBullets.clear();
    ClearDungeon();
    bulletLights.clear();
    dungeonEntrances.clear();
    
    RemoveAllVegetation();

    if (terrainMesh.vertexCount > 0) UnloadMesh(terrainMesh); //unload mesh and heightmap when switching levels. if they exist
    if (heightmap.data != nullptr) UnloadImage(heightmap); 
    isDungeon = false;

}



