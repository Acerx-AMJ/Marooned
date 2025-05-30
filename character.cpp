#include "Character.h"

Character::Character(Vector3 pos, Texture2D* tex, int fw, int fh, int frames, float speed, float scl)
    : position(pos), texture(tex), frameWidth(fw), frameHeight(fh),
      currentFrame(0), maxFrames(frames), animationTimer(0), animationSpeed(speed), scale(scl)
{}

void Character::Update(float deltaTime) {
    animationTimer += deltaTime;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0;
        currentFrame = (currentFrame + 1) % maxFrames;
    }
}

void Character::Draw(Camera3D camera) {
    Rectangle sourceRec = {
        (float)(currentFrame * frameWidth),
        (float)texture->height,
        (float)frameWidth,
        -(float)frameHeight // flip vertically
    };

    Vector2 size = { frameWidth * scale, frameHeight * scale };

    DrawBillboardRec(camera, *texture, sourceRec, position, size, WHITE);
}
