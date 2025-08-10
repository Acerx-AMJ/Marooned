#include "camera_system.h"
#include "raymath.h"
#include "input.h" 
#include "world.h"

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

    freeRig = playerRig; // start free cam matching player
}

void CameraSystem::SnapAllToPlayer() {
    Vector3 camPos = {player.position.x, 200, player.position.z};
    playerRig.cam.position = camPos;
    freeRig.cam   = playerRig.cam;
    freeRig.fov   = playerRig.fov;
    freeRig.yaw   = playerRig.yaw;
    freeRig.pitch = playerRig.pitch;
}

void CameraSystem::AttachToPlayer(const Vector3& pos, const Vector3& forward) {
    // Simple follow; you can add head-bob/weapon sway offsets here
    playerRig.cam.position = pos;
    playerRig.cam.target   = Vector3Add(pos, forward);
}

void CameraSystem::SetMode(CamMode m) { mode = m; }
CamMode CameraSystem::GetMode() const { return mode; }

void CameraSystem::SetFOV(float fov) {
    playerRig.fov = fov; 
    freeRig.fov   = fov;
}

void CameraSystem::SetFarClip(float farClip) {
    playerRig.farClip = farClip;
    freeRig.farClip   = farClip;
}

Camera3D& CameraSystem::Active() {
    if (mode == CamMode::Player) { playerRig.cam.fovy = playerRig.fov; return playerRig.cam; }
    else                         { freeRig.cam.fovy   = freeRig.fov;   return freeRig.cam;  }
}
const Camera3D& CameraSystem::Active() const {
    return (mode == CamMode::Player) ? playerRig.cam : freeRig.cam;
}

void CameraSystem::Shake(float mag, float dur) { shakeMag = mag; shakeTime = dur; }

void CameraSystem::ApplyShake(float dt) {
    if (shakeTime <= 0.f) return;
    shakeTime -= dt;
    float t = shakeTime;
    Vector3 jitter = { (GetRandomValue(-100,100)/100.f)*shakeMag,
                       (GetRandomValue(-100,100)/100.f)*shakeMag,
                       (GetRandomValue(-100,100)/100.f)*shakeMag };
    if (mode == CamMode::Player) playerRig.cam.position = Vector3Add(playerRig.cam.position, jitter);
    else                         freeRig.cam.position   = Vector3Add(freeRig.cam.position, jitter);
}

void CameraSystem::Update(float dt) {
    if (mode == CamMode::Player) UpdatePlayerCam(dt);
    else                         UpdateFreeCam(dt);
    ApplyShake(dt);
}


void CameraSystem::UpdatePlayerCam(float dt) {
    // 1) Choose base position (boat vs ground)
    Vector3 basePos = pv.onBoard
        ? Vector3Add(pv.boatPos, Vector3{0, 200.0f, 0})
        : Vector3Add(pv.position, Vector3{0, 0, 0});

    // 2) Swimming lowers the camera (keeps your old magic value)
    float swimYOffset = player.isSwimming ? -40.0f : 1.5f;  // swimming lowers the camera
    if (!pv.onBoard && pv.isSwimming) basePos.y += swimYOffset;

    // 3) Build forward from player yaw/pitch
    float yawRad   = DEG2RAD * pv.yawDeg;
    float pitchRad = DEG2RAD * pv.pitchDeg;
    Vector3 forward = {
        cosf(pitchRad) * sinf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * cosf(yawRad)
    };
    Vector3 target = Vector3Add(basePos, forward);

    // 4) Apply to the rig
    playerRig.cam.position = basePos;
    playerRig.cam.target   = target;

    // 5) Per-scene far clip (you were doing this in render)
    float farClip = isDungeon ? 10000.0f : 50000.0f; // use your global or pass it in
    SetFarClip(farClip);
}

void CameraSystem::UpdateFreeCam(float dt) {
    // WASD + mouse-look example (change to your input stack)
    const float speed = (IsKeyDown(KEY_LEFT_SHIFT) ? 900.f : 400.f);
    Vector3 f = Vector3Normalize(Vector3Subtract(freeRig.cam.target, freeRig.cam.position));
    Vector3 r = Vector3Normalize(Vector3CrossProduct(f, {0,1,0}));

    Vector3 move{0,0,0};
    if (IsKeyDown(KEY_W)) move = Vector3Add(move, f);
    if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, f);
    if (IsKeyDown(KEY_A)) move = Vector3Subtract(move, r);
    if (IsKeyDown(KEY_D)) move = Vector3Add(move, r);

    if (Vector3Length(move) > 0) {
        move = Vector3Scale(Vector3Normalize(move), speed * dt);
        freeRig.cam.position = Vector3Add(freeRig.cam.position, move);
        freeRig.cam.target   = Vector3Add(freeRig.cam.target, move);
    }

    // Mouse look (basic)
    Vector2 delta = GetMouseDelta();
    float sens = 0.12f;
    freeRig.yaw   -= delta.x * sens;
    freeRig.pitch += -delta.y * sens;
    freeRig.pitch = Clamp(freeRig.pitch, -89.f, 89.f);

    Vector3 dir{
        cosf(DEG2RAD*freeRig.pitch) * sinf(DEG2RAD*freeRig.yaw),
        sinf(DEG2RAD*freeRig.pitch),
        cosf(DEG2RAD*freeRig.pitch) * cosf(DEG2RAD*freeRig.yaw)
    };
    freeRig.cam.target = Vector3Add(freeRig.cam.position, dir);
    freeRig.cam.fovy   = freeRig.fov;
}
