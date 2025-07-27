#include "utilities.h"
#include <iostream> // for DebugPrintVector

// ───── Math ─────────────────────────────────────────────


float RandomFloat(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
}

// ───── Vector3 ───────────────────────────────────────────

Vector3 LerpVec3(Vector3 a, Vector3 b, float t) {
    return Vector3{
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t),
        Lerp(a.z, b.z, t)
    };
}

void DebugPrintVector(const Vector3& v) {
    std::cout << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}
