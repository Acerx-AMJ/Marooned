#include "utilities.h"
#include <iostream> // for DebugPrintVector

// ───── Math ─────────────────────────────────────────────

float Clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }

float RandomFloat(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
}


//Lerp healthbar value to target
float LerpExp(float current, float target, float lambda, float dt) {
    if (lambda <= 0.0f) return target;
    float a = 1.0f - expf(-lambda * dt);
    return current + (target - current) * a;
}

Vector2 LerpV2(Vector2 a, Vector2 b, float t) {
    return { a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t };
}

// ───── Vector3 ───────────────────────────────────────────

Vector3 LerpVec3(Vector3 a, Vector3 b, float t) {
    return Vector3{
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t)
    };
}


Color ColorLerpFast(Color a, Color b, float t) {
    t = Clamp01(t);
    return Color{
        (unsigned char)(a.r + (b.r - a.r)*t),
        (unsigned char)(a.g + (b.g - a.g)*t),
        (unsigned char)(a.b + (b.b - a.b)*t),
        (unsigned char)(a.a + (b.a - a.a)*t)
    };
}

// helper to lerp between two Colors (normalized to 0..1 first)
Color LerpColor(Color a, Color b, float t) {
    Vector3 va = { a.r / 255.0f, a.g / 255.0f, a.b / 255.0f };
    Vector3 vb = { b.r / 255.0f, b.g / 255.0f, b.b / 255.0f };

    Vector3 v = {
        va.x + (vb.x - va.x) * t,
        va.y + (vb.y - va.y) * t,
        va.z + (vb.z - va.z) * t
    };

    return Color{
        (unsigned char)Clamp(v.x * 255.0f, 0.0f, 255.0f),
        (unsigned char)Clamp(v.y * 255.0f, 0.0f, 255.0f),
        (unsigned char)Clamp(v.z * 255.0f, 0.0f, 255.0f),
        255
    };
}

void DebugPrintVector(const Vector3& v) {
    std::cout << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}
