#include "sound_manager.h"
#include <iostream>
#include "raymath.h"
std::vector<std::string> footstepKeys;
SoundManager& SoundManager::GetInstance() {
    static SoundManager instance;
    return instance;
}



void SoundManager::LoadSound(const std::string& name, const std::string& filePath) {
    Sound sound = ::LoadSound(filePath.c_str());
    sounds[name] = sound;
}

Sound SoundManager::GetSound(const std::string& name) {
    if (sounds.count(name)) {
        return sounds[name];
    }
    std::cerr << "Sound not found: " << name << std::endl;
    return {0}; // Empty sound
}

void SoundManager::Play(const std::string& name) {
    if (sounds.count(name)) {
        PlaySound(sounds[name]);
    }
}

void SoundManager::PlaySoundAtPosition(const std::string& soundName, const Vector3& soundPos, const Vector3& listenerPos, float listenerYaw, float maxDistance) {
    if (sounds.find(soundName) == sounds.end()) {
        std::cerr << "Sound not found: " << soundName << std::endl;
        return;
    }

    Vector3 delta = Vector3Subtract(soundPos, listenerPos);
    float distance = Vector3Length(delta);
    if (distance > maxDistance) return;

    float volume = Clamp(1.0f - (distance / maxDistance), 0.0f, 1.0f);

    // Convert yaw to radians ////////////FAILED 3D audio attempt. 
   //float yawRad = DEG2RAD * listenerYaw;
    // float yawRad = DEG2RAD * -listenerYaw;  // flip sign here
    // // Rotate world delta into local space
    // // localX = right/left relative to where player is looking
    // float localX = delta.x * cosf(yawRad) + delta.z * sinf(yawRad);
    // float localZ = delta.z * cosf(yawRad) - delta.x * sinf(yawRad);

    // // Now compute pan from localX (side) and localZ (front/back)
    // float pan = Clamp(localX / (fabs(localZ) + 1.0f), -1.0f, 1.0f);

    // // Optional: Scale the pan for less extreme stereo effect
    // float panScale = 0.01f;
    // float finalPan = Clamp(pan * panScale, -1.0f, 0.0f);


    Sound sound = sounds[soundName];
    // StopSound(sound);
    SetSoundVolume(sound, volume);
    // SetSoundPan(sound, finalPan);
    PlaySound(sound);


}



// void SoundManager::PlaySoundAtPosition(const std::string& soundName, const Vector3& soundPos, const Vector3& listenerPos, float maxDistance) {
//     if (sounds.find(soundName) == sounds.end()) {
//         std::cerr << "Sound not found: " << soundName << std::endl;
//         return;
//     }

//     float distance = Vector3Distance(soundPos, listenerPos);
//     float volume = Clamp(1.0f - (distance / maxDistance), 0.0f, 1.0f);

//     if (volume < 0.01f) return; // Skip inaudible

//     // --- Calculate pan ---
//     Vector3 delta = Vector3Subtract(soundPos, listenerPos);
//     float pan = Clamp(delta.x, -1.0f, 1.0f); // Simple horizontal pan

//     Sound sound = sounds[soundName];
//     SetSoundVolume(sound, volume);
//     SetSoundPan(sound, pan);
//     PlaySound(sound);
// }



SoundManager::~SoundManager() {
    UnloadAll();
}

void SoundManager::UnloadAll() {
    for (auto& [name, sound] : sounds) {
        ::UnloadSound(sound);
    }
    sounds.clear();
}
