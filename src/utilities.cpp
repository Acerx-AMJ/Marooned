#include "utilities.h"
#include <iostream> // for DebugPrintVector

// ───── Math ─────────────────────────────────────────────

float Clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }

float RandomFloat(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
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

void DebugPrintVector(const Vector3& v) {
    std::cout << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}
