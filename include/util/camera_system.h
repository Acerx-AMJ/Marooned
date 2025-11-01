#pragma once
#include "raylib.h"

struct PlayerView {
    Vector3 position;
    float yawDeg;      // player.rotation.y
    float pitchDeg;    // player.rotation.x
    bool isSwimming;
    bool onBoard;
    Vector3 boatPos;   // only used if onBoard
    float camDipY;

};

struct CameraRig {
    Camera3D cam{};
    float yaw = 0.f;
    float pitch = 0.f;
    float fov = 45.f;
    float nearClip = 60.0f;      // matches your BeginCustom3D
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

    // zoom/FOV/states
    void SetFOV(float fov);
    void SetFarClip(float farClip);
    void Shake(float magnitude, float duration);

    bool aspectSquare = false;
    // read-only access for rendering
    Camera3D& Active();
    const Camera3D& Active() const;
    void BeginCustom3D(const Camera3D& cam, float nearClip, float farClip);



private:
    CameraSystem() = default;


    // store the latest player snapshot
    PlayerView pv{};
    void UpdatePlayerCam();
    void ApplyShake(float dt);
    CameraRig playerRig{};

    float shakeTime = 0.f, shakeMag = 0.f;
};
