// utilities.h
#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>

// Math
float RandomFloat(float min, float max);
float Clamp01(float x);
float LerpExp(float current, float target, float lambda, float dt); 
// Vectors
Vector2 LerpV2(Vector2 a, Vector2 b, float t);
Vector3 LerpVec3(Vector3 a, Vector3 b, float t);
void DebugPrintVector(const Vector3& v);
//color
Color ColorLerpFast(Color a, Color b, float t);
Color LerpColor(Color a, Color b, float t);
Vector3 Limit(Vector3 v, float maxLen);

//xz steering
Vector3 NormalizeXZ(Vector3 v);
Vector3 RandomPointOnRingXZ(const Vector3& center, float minR, float maxR);
float DistXZ(const Vector3& a, const Vector3& b);
inline float Rand01() { return (float)GetRandomValue(0, 1000) / 1000.0f; }