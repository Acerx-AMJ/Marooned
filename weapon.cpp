#include "weapon.h"
#include "raymath.h"
#include <iostream>
#include "rlgl.h"
#include "bullet.h"


void Weapon::Fire(Camera& camera) {
    if (GetTime() - lastFired >= fireCooldown) {
        PlaySound(fireSound);
        recoil = recoilAmount;
        lastFired = GetTime();
        flashTimer = flashDuration;


        //offset bulletOrigin to weapon position. 
        Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, { 0, 1, 0 }));
        Vector3 camUp = { 0, 1, 0 };

        // Offsets in local space
        float forwardOffset = -50.0f;
        float sideOffset = 30.0f;
        float verticalOffset = -40.0f; // down

        // Final origin for bullets in world space
        Vector3 bulletOrigin = camera.position;
        bulletOrigin = Vector3Add(bulletOrigin, Vector3Scale(camForward, forwardOffset));
        bulletOrigin = Vector3Add(bulletOrigin, Vector3Scale(camRight, sideOffset));
        bulletOrigin = Vector3Add(bulletOrigin, Vector3Scale(camUp, verticalOffset));

        FireBlunderbuss(
            bulletOrigin,
            camForward,
            3.0f,    // spreadDegrees (tweak this!)
            6,        // pelletCount
            1500.0f,   // bulletSpeed
            1.2f      // lifetimeSeconds
        );
    }


}

void Weapon::Update(float deltaTime) {
    if (recoil > 0.0f) {
        recoil -= recoilRecoverySpeed * deltaTime;
        if (recoil < 0.0f) recoil = 0.0f;
    }

    if (flashTimer > 0.0f) {
        flashTimer -= deltaTime;
        if (flashTimer < 0.0f) flashTimer = 0.0f;
    }
}

void Weapon::Draw(const Camera& camera) {
    Matrix lookAt = MatrixLookAt(camera.position, camera.target, { 0, 1, 0 });
    Matrix gunRotation = MatrixInvert(lookAt);
    Quaternion q = QuaternionFromMatrix(gunRotation);

    float angle = 2.0f * acosf(q.w);
    float angleDeg = angle * RAD2DEG;
    float sinTheta = sqrtf(1.0f - q.w * q.w);
    Vector3 axis = (sinTheta < 0.001f) ? Vector3{1, 0, 0} : Vector3{ q.x / sinTheta, q.y / sinTheta, q.z / sinTheta };

    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, { 0, 1, 0 }));
    Vector3 camUp = { 0, 1, 0 };

    float dynamicForward = forwardOffset - recoil;

    Vector3 gunPos = camera.position;
    gunPos = Vector3Add(gunPos, Vector3Scale(camForward, dynamicForward));
    gunPos = Vector3Add(gunPos, Vector3Scale(camRight, sideOffset));
    gunPos = Vector3Add(gunPos, Vector3Scale(camUp, verticalOffset));

    if (flashTimer > 0.0f) {
        Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 muzzlePos = Vector3Add(gunPos, Vector3Scale(camForward, 40.0f));
        
        float flashSize = 60.0f; // tweak as needed
        rlDisableDepthMask();
        //rlDisableDepthTest();
        DrawBillboard(camera, muzzleFlashTexture, muzzlePos, flashSize, WHITE);
        rlEnableDepthMask();
        //rlDisableDepthTest();
    }

    DrawModelEx(model, gunPos, axis, angleDeg, scale, WHITE);


}
