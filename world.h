// world.h
#pragma once
#include "raylib.h"
#include "player.h"
#include <vector>
#include "character.h"
#include "bullet.h"
#include "vegetation.h"
#include "decal.h"
#include "level.h"

enum class GameState {
    Menu,
    Playing,
    Quit
};

extern bool controlPlayer;
extern bool isDungeon;
extern float dungeonPlayerHeight;
extern float floorHeight;
extern float wallHeight;
extern unsigned char* heightmapPixels;
extern Vector3 startPosition;
extern Vector3 playerSpawnPoint;
extern int selectedOption; // 0 = Start, 1 = Quit
extern int levelIndex;
extern Player player;
extern Vector3 boatPosition;
extern float fadeToBlack;
extern float vignetteIntensity;
extern float vignetteFade;
extern float boatSpeed;
extern float waterHeightY;
extern const float TREE_HEIGHT_RATIO;
extern const float BUSH_HEIGHT_RATIO;
extern float tileSize;
extern bool isFading;
extern float fadeSpeed; // units per second
extern bool fadeIn;     // true = fade in (to black), false = fade out (to visible)


extern GameState currentGameState;
extern std::vector<Character> raptors;
extern std::vector<Character*> raptorPtrs;
extern std::vector<Character> skeletons;
extern std::vector<Character*> skeletonPtrs;
extern std::vector<DungeonEntrance> dungeonEntrances;
extern std::vector<Bullet> activeBullets;
extern std::vector<Decal> decals;

float GetHeightAtWorldPosition(Vector3 position, Image heightmap, Vector3 terrainScale);
void removeAllRaptors();
void generateRaptors(int amount, Vector3 centerPos, float radius);
void regenerateRaptors(int amount, Vector3 centerPos, float radius);
bool CheckTreeCollision(const TreeInstance& tree, const Vector3& playerPos);
void ResolveTreeCollision(const TreeInstance& tree, Vector3& playerPos);