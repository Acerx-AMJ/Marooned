#pragma once
#include "raylib.h"

enum class InputMode {
    KeyboardMouse,
    Gamepad
};

extern InputMode currentInputMode;

void UpdateInputMode();
void debugControls();
void UpdateCameraWithGamepad(Camera3D& camera);
