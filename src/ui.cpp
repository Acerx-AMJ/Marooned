// ui.cpp
#include "ui.h"
#include <cmath>
#include "raymath.h"
#include "level.h"
#include "resourceManager.h"
#include "weapon.h"
#include "world.h"

void DrawMagicIcon(){
    Texture2D currentTexture;

    if (magicStaff.magicType == MagicType::Fireball) {
        currentTexture = R.GetTexture("fireIcon");
    } else if (magicStaff.magicType == MagicType::Iceball) {
        currentTexture = R.GetTexture("iceIcon");
    }

    int targetSize = 64;
    int marginX = 314; // distance from right screen edge
    int marginY = GetScreenHeight() - targetSize - 16;
    // Source rect: crop entire original texture
    Rectangle src = { 0.0f, 0.0f, (float)currentTexture.width, (float)currentTexture.height };

    // Destination rect: bottom-left corner, scaled to 64x64
    Rectangle dest = {
        (float)marginX,
        (float)marginY,
        (float)targetSize,
        (float)targetSize
    };

    Vector2 origin = { 0.0f, 0.0f }; // top-left origin

    DrawTexturePro(currentTexture, src, dest, origin, 0.0f, WHITE);
}



void DrawHealthBar(const Player& player) {
    float healthPercent = (float)player.currentHealth / player.maxHealth;
    healthPercent = Clamp(healthPercent, 0.0f, 1.0f);

    int barWidth = 300;
    int barHeight = 30;
    int barX = GetScreenWidth() / 3 - barWidth / 2;
    int barY = GetScreenHeight() - 80;

    Rectangle healthBarCurrent = { (float)barX, (float)barY, (float)(barWidth * healthPercent), (float)barHeight };

    DrawRectangleLines(barX - 1, barY - 1, barWidth + 2, barHeight + 2, WHITE);

    Color barColor = WHITE;
    if (healthPercent < 0.25f) {
        float pulse = sin(GetTime() * 10.0f) * 0.5f + 0.5f;
        barColor = ColorLerp(WHITE, RED, pulse);
    }

    DrawRectangleRec(healthBarCurrent, barColor);
}

void DrawStaminaBar(const Player& player) {
    float percent = player.stamina / player.maxStamina;
    percent = Clamp(percent, 0.0f, 1.0f);

    int width = 300;
    int height = 10;
    int x = GetScreenWidth() / 3 - width / 2;
    int y = GetScreenHeight() - 40;

    Rectangle barCurrent = { (float)x, (float)y, (float)(width * percent), (float)height };

    DrawRectangleLines(x - 1, y - 1, width + 2, height + 2, DARKGRAY);
    Color color = ColorLerp((Color){50, 50, 150, 255}, BLUE, percent);
    DrawRectangleRec(barCurrent, color);
}

void DrawManaBar(const Player& player) {
    float percent = (float)player.currentMana / player.maxMana;
    percent = Clamp(percent, 0.0f, 1.0f);

    int width = 300;
    int height = 10;
    int x = GetScreenWidth() / 3 - width / 2;
    int y = GetScreenHeight() - 25;

    Rectangle barCurrent = { (float)x, (float)y, (float)(width * percent), (float)height };

    DrawRectangleLines(x - 1, y - 1, width + 2, height + 2, DARKBLUE);
    Color color = ColorLerp((Color){30, 30, 60, 255}, (Color){100, 100, 255, 255}, percent);
    DrawRectangleRec(barCurrent, color);
}

void DrawMenu(int selectedOption, int levelIndex) {
    ClearBackground(BLACK);
    Texture2D backDrop = R.GetTexture("backDrop");
    DrawTexturePro(
        backDrop,
        Rectangle{ 0, 0, (float)backDrop.width, (float)backDrop.height },
        Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
        Vector2{ 0, 0 },
        0.0f,
        WHITE
    );

    //float middle = GetScreenWidth()/2 - 150;
    const char* title = "MAROONED";
    int fontSize = 60;
    int titleX = GetScreenWidth() / 2 - MeasureText(title, fontSize) / 2;
    DrawText(title, titleX, 180, fontSize, GREEN);

    DrawText(selectedOption == 0 ? "> Start" : "  Start", titleX, 280, 30, WHITE);
    
    DrawText(
        TextFormat("%s Level: %s", selectedOption == 1 ? ">" : " ", levels[levelIndex].name.c_str()),
        titleX, 330, 30, YELLOW
    );

    DrawText(selectedOption == 2 ? "> Quit" : "  Quit", titleX, 380, 30, WHITE);
}

void UpdateMenu(Camera& camera){
    //Main Menu - level select 
    if (currentGameState == GameState::Menu) {
        if (IsKeyPressed(KEY_ESCAPE)) currentGameState = GameState::Playing;
        if (IsKeyPressed(KEY_UP)) selectedOption = (selectedOption - 1 + 3) % 3;
        if (IsKeyPressed(KEY_DOWN)) selectedOption = (selectedOption + 1) % 3;

        if (IsKeyPressed(KEY_ENTER)) {
            if (selectedOption == 0) {
                InitLevel(levels[levelIndex], camera);
                currentGameState = GameState::Playing;
            } else if (selectedOption == 1) {
                levelIndex = (levelIndex + 1) % levels.size(); // Cycle through levels
            } else if (selectedOption == 2) {
                currentGameState = GameState::Quit;
            }
        }

    }
}

void DrawTimer(float ElapsedTime){
    int minutes = (int)(ElapsedTime / 60.0f);
    int seconds = (int)ElapsedTime % 60;

    char buffer[16];
    sprintf(buffer, "Time: %02d:%02d", minutes, seconds);

    DrawText(buffer, GetScreenWidth()-150, 30, 20, WHITE); 
}