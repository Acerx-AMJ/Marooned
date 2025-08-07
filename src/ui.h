// ui.h
#pragma once
#include "raylib.h"
#include "player.h" // assuming you need access to player's stats

void DrawMagicIcon();
void DrawHealthBar(const Player& player);
void DrawStaminaBar(const Player& player);
void DrawManaBar(const Player& player);
void DrawMenu(int selectedOption, int levelIndex); 
void DrawTimer(float ElapsedTime);
void UpdateMenu(Camera& camera);