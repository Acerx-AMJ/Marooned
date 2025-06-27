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


    Sound sound = sounds[soundName];
    // StopSound(sound);
    SetSoundVolume(sound, volume);
    // SetSoundPan(sound, finalPan);
    PlaySound(sound);


}



SoundManager::~SoundManager() {
    UnloadAll();
}

void SoundManager::UnloadAll() {
    for (auto& [name, sound] : sounds) {
        ::UnloadSound(sound);
    }
    sounds.clear();
}
