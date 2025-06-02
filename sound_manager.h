#pragma once

#include "raylib.h"
#include <map>
#include <string>
#include <vector>

class SoundManager {
public:
    static SoundManager& GetInstance(); // Singleton


    void LoadSound(const std::string& name, const std::string& filePath);
    Sound GetSound(const std::string& name);
    void Play(const std::string& name);
    void UnloadAll();

private:
    std::map<std::string, Sound> sounds;

    SoundManager() = default;
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
};
