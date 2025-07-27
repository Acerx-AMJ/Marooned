// ui.h
#pragma once
#include "raylib.h"
#include "player.h" // assuming you need access to player's stats

void DrawHealthBar(const Player& player);
void DrawStaminaBar(const Player& player);
void DrawManaBar(const Player& player);
void DrawMenu(Texture2D& backDrop, int selectedOption, int levelIndex); 
void DrawTimer(float ElapsedTime);