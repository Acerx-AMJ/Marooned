#include "util/camera_system.h"

#include "rlgl.h"
#include "world/world.h"

CameraSystem& CameraSystem::Get() {
    static CameraSystem instance;
    return instance;
}

void CameraSystem::SyncFromPlayer(const PlayerView& view) {
    pv = view;
}

void CameraSystem::Init(const Vector3& startPos) {
    playerRig.cam.position = startPos;
    playerRig.cam.target   = Vector3Add(startPos, {0,0,1});
    playerRig.cam.up       = {0,1,0};
    playerRig.cam.fovy     = playerRig.fov;
    playerRig.cam.projection = CAMERA_PERSPECTIVE;
}

void CameraSystem::AttachToPlayer(const Vector3& pos, const Vector3& forward) {
    // Simple follow; you can add head-bob/weapon sway offsets here
    playerRig.cam.position = pos;
    playerRig.cam.target   = Vector3Add(pos, forward);
}

void CameraSystem::SetFOV(float fov) {
    playerRig.fov = fov;
}

void CameraSystem::SetFarClip(float farClip) {
    playerRig.farClip = farClip;
}

Camera3D& CameraSystem::Active() {
   playerRig.cam.fovy = playerRig.fov;
   return playerRig.cam;
}
const Camera3D& CameraSystem::Active() const {
    return playerRig.cam;
}

void CameraSystem::Shake(float mag, float dur) { shakeMag = mag; shakeTime = dur; }

void CameraSystem::ApplyShake(float dt) {
    if (shakeTime <= 0.f) return;
    shakeTime -= dt;
    Vector3 jitter = { (GetRandomValue(-100,100)/100.f)*shakeMag,
                       (GetRandomValue(-100,100)/100.f)*shakeMag,
                       (GetRandomValue(-100,100)/100.f)*shakeMag };
    playerRig.cam.position = Vector3Add(playerRig.cam.position, jitter);
}

void CameraSystem::Update(float dt) {
    UpdatePlayerCam();
    ApplyShake(dt);
}



// Exponential smoothing helper
inline float SmoothStepExp(float current, float target, float speed, float dt) {
    float a = 1.0f - expf(-speed * dt);
    return current + (target - current) * a;
}


void CameraSystem::UpdatePlayerCam() {
    Vector3 basePos = pv.onBoard
        ? Vector3Add(pv.boatPos, Vector3{0, 200.0f, 0})
        : pv.position;

    if (player.isSwimming && !pv.onBoard) basePos.y -= 40.0f;

    float yawRad   = DEG2RAD * pv.yawDeg;
    float pitchRad = DEG2RAD * pv.pitchDeg;

    Vector3 forward = {
        cosf(pitchRad) * sinf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * cosf(yawRad)
    };

    playerRig.cam.position = basePos;
    playerRig.cam.target   = Vector3Add(basePos, forward);

    // keep the angles cached so other rigs can copy
    playerRig.yaw   = pv.yawDeg;
    playerRig.pitch = pv.pitchDeg;

    SetFarClip(isDungeon ? 16000.0f : 50000.0f);
}

void CameraSystem::BeginCustom3D(const Camera3D& cam, float nearClip, float farClip) {
    rlDrawRenderBatchActive();

    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    Matrix proj = MatrixPerspective(DEG2RAD * cam.fovy,
                                (float)GetScreenWidth()/(float)GetScreenHeight(),
                                nearClip, farClip);
    rlMultMatrixf(MatrixToFloat(proj));

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(cam.position, cam.target, cam.up);
    rlMultMatrixf(MatrixToFloat(view));
}
