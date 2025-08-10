#pragma once
#include "raylib.h"

enum class CamMode { Player, Free };

struct PlayerView {
    Vector3 position;
    float yawDeg;      // player.rotation.y
    float pitchDeg;    // player.rotation.x
    bool isSwimming;
    bool onBoard;
    Vector3 boatPos;   // only used if onBoard
};

struct CameraRig {
    Camera3D cam{};
    float yaw = 0.f;
    float pitch = 0.f;
    float fov = 45.f;
    float nearClip = 60.f;      // matches your BeginCustom3D
    float farClip = 50000.f;
    Vector3 velocity{0,0,0};    // free-cam movement
};

class CameraSystem {
public:
    static CameraSystem& Get();

    void Init(const Vector3& startPos);         // called once after resources load
    void SyncFromPlayer(const PlayerView& view);
    void Update(float dt);                      // handles input + smoothing
    void AttachToPlayer(const Vector3& pos, const Vector3& forward); // update target from player
    void SetMode(CamMode m);
    void SnapAllToPlayer();   // copies player rig → free rig
    CamMode GetMode() const;

    // zoom/FOV/states
    void SetFOV(float fov);
    void SetFarClip(float farClip);
    void Shake(float magnitude, float duration);

    bool IsPlayerMode() const { return GetMode() == CamMode::Player; }

    // read-only access for rendering
    Camera3D& Active();
    const Camera3D& Active() const;

private:
    CameraSystem() = default;


    // store the latest player snapshot
    PlayerView pv{};
    void UpdatePlayerCam(float dt);
    void UpdateFreeCam(float dt);
    void ApplyShake(float dt);

    CamMode mode = CamMode::Player;
    CameraRig playerRig{};
    CameraRig freeRig{};
    float shakeTime = 0.f, shakeMag = 0.f;
};
