// utilities.h
#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>

// Math
float RandomFloat(float min, float max);
float Clamp01(float x);
// Vectors
Vector2 LerpV2(Vector2 a, Vector2 b, float t);
Vector3 LerpVec3(Vector3 a, Vector3 b, float t);
void DebugPrintVector(const Vector3& v);
//color
Color ColorLerpFast(Color a, Color b, float t);

