#include "utilities.h"
#include <iostream> // for DebugPrintVector


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

Vector3 NormalizeXZ(Vector3 v) {
    v.y = 0.0f;
    float m2 = v.x*v.x + v.z*v.z;
    if (m2 <= 1e-6f) return {0,0,0};
    float inv = 1.0f / sqrtf(m2);
    return { v.x*inv, 0.0f, v.z*inv };
}

Vector3 Limit(Vector3 v, float maxLen) {
    float m2 = v.x*v.x + v.z*v.z;
    if (m2 > maxLen*maxLen) {
        float inv = maxLen / sqrtf(m2);
        v.x *= inv; v.z *= inv;
    }
    v.y = 0.0f;
    return v;
}



float DistXZ(const Vector3& a, const Vector3& b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return sqrtf(dx*dx + dz*dz);
}

// Pick a random point on a ring [minR, maxR] around 'center' (XZ only)
Vector3 RandomPointOnRingXZ(const Vector3& center, float minR, float maxR) {
    float angle = Rand01() * 2.0f * PI;
    float r     = minR + Rand01() * (maxR - minR);
    Vector3 p{ center.x + sinf(angle) * r, 0.0f, center.z + cosf(angle) * r };
    return p;
}



void DebugPrintVector(const Vector3& v) {
    std::cout << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}
