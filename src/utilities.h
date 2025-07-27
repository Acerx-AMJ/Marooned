// utilities.h
#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>

// Math
float RandomFloat(float min, float max);

// Vectors
Vector3 LerpVec3(Vector3 a, Vector3 b, float t);
void DebugPrintVector(const Vector3& v);
