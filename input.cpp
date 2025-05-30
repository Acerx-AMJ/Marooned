#include "input.h"
#include "raymath.h"

InputMode currentInputMode = InputMode::KeyboardMouse;


void UpdateCameraWithGamepad(Camera3D& camera) {
    if (!IsGamepadAvailable(0)) return;

    float lookSensitivity = 0.03f;

    float yawInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    float pitchInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

    float yaw = -yawInput * lookSensitivity;
    float pitch = -pitchInput * lookSensitivity;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    // Apply yaw (rotate around up)
    Matrix yawRot = MatrixRotate(camera.up, yaw);
    forward = Vector3Transform(forward, yawRot);

    // Apply pitch (rotate around right)
    Matrix pitchRot = MatrixRotate(right, pitch);
    forward = Vector3Transform(forward, pitchRot);

    // Move camera position
    float moveSpeed = 1.0f;
    Vector2 stick = {
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)
    };

    camera.position = Vector3Add(camera.position, Vector3Scale(right, stick.x * moveSpeed));
    camera.position = Vector3Add(camera.position, Vector3Scale(forward, -stick.y * moveSpeed));
    camera.target = Vector3Add(camera.position, forward);
}

void UpdateInputMode() {
    // Check gamepad activity
    if (IsGamepadAvailable(0)) {
        float lx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float ly = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        float rx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        float ry = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

        if (fabsf(lx) > 0.1f || fabsf(ly) > 0.1f || fabsf(rx) > 0.1f || fabsf(ry) > 0.1f) {
            currentInputMode = InputMode::Gamepad;
            return;
        }

        for (int b = 0; b <= GAMEPAD_BUTTON_MIDDLE_RIGHT; b++) {
            if (IsGamepadButtonPressed(0, b)) {
                currentInputMode = InputMode::Gamepad;
                return;
            }
        }
    }

    // Check mouse/keyboard activity
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) ||
        IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ||
        GetMouseDelta().x != 0 || GetMouseDelta().y != 0) {
        currentInputMode = InputMode::KeyboardMouse;
    }
}
