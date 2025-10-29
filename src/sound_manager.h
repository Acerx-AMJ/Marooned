#pragma once

#include "raylib.h"
#include <map>
#include <string>

class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    static SoundManager& Get(); // Singleton

    void LoadSound(const std::string& name, const std::string& filePath);
    void LoadMusic(const std::string& name, const std::string& filePath);
    void LoadSounds();
    void UnloadAll();

    void PlayMusic(const std::string& name, float volume = 1.0f);
    void Play(const std::string& name);
    void PlaySoundAtPosition(const std::string& soundName, const Vector3& soundPos, const Vector3& listenerPos, float listenerYaw, float maxDistance);
    
    void Update(); // Call this every frame
    Music& GetMusic(const std::string& name);
    Sound& GetSound(const std::string& name);

private:
    std::map<std::string, Sound> sounds;
    std::map<std::string, Music> musicTracks;
};
