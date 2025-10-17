#pragma once

#include "raylib.h"
#include <map>
#include <string>
#include <vector>

class SoundManager {
public:
    static SoundManager& GetInstance(); // Singleton
    void PlaySoundAtPosition(const std::string& soundName, const Vector3& soundPos, const Vector3& listenerPos, float listenerYaw, float maxDistance);
    void LoadMusic(const std::string& name, const std::string& filePath);
    void PlayMusic(const std::string& name, float volume = 1.0f);
    void Update(); // Call this every frame
    Music& GetMusic(const std::string& name);
    void LoadSound(const std::string& name, const std::string& filePath);
    Sound GetSound(const std::string& name);
    void Play(const std::string& name);
    void Stop(const std::string& name);
    bool IsPlaying(const std::string& name) const;
    void LoadSounds();
    void UnloadAll();

private:
    std::map<std::string, Sound> sounds;
    std::map<std::string, Music> musicTracks;

    SoundManager() = default;
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
};
