#pragma once
#include "raylib.h"

enum class InputMode {
    KeyboardMouse,
    Gamepad
};

extern InputMode currentInputMode;


void UpdateInputMode();
void debugControls(Camera& camera);
void UpdateCameraWithGamepad(Camera3D& camera);
void HandleGamepadInput(float deltaTime);
void HandleKeyboardInput(float deltaTime);
void HandleMouseLook(float deltaTime);
void HandleStickLook(float deltaTime);
