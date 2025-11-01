#include "util/sound_manager.h"

// Includes

#include <filesystem>
#include <iostream>
#include "raymath.h"

// Constructors

SoundManager::~SoundManager() {
    UnloadAll();
}

SoundManager& SoundManager::Get() {
    static SoundManager instance;
    return instance;
}

// Loading functions

void SoundManager::LoadSound(const std::string& name, const std::string& filePath) {
    Sound sound = ::LoadSound(filePath.c_str());
    sounds[name] = sound;
}

void SoundManager::LoadMusic(const std::string& name, const std::string& filePath) {
    Music music = LoadMusicStream(filePath.c_str());
    musicTracks[name] = music;
}

void SoundManager::LoadSounds() {
    for (const auto& file : std::filesystem::directory_iterator("assets/sounds/")) {
        LoadSound(file.path().stem().string(), file.path().string());
    }

    for (const auto& file : std::filesystem::directory_iterator("assets/music/")) {
        LoadMusic(file.path().stem().string(), file.path().string());
    }
}

void SoundManager::UnloadAll() {
    for (auto& [name, sound] : sounds) {
        ::UnloadSound(sound);
    }
    for (auto& [name, music] : musicTracks){
        ::UnloadMusicStream(music);
    }
}

// Play functions

void SoundManager::PlayMusic(const std::string& name, float volume) {
    if (musicTracks.find(name) == musicTracks.end()) {
        std::cerr << "Music not found: " << name << std::endl;
        return;
    }

    Music& music = musicTracks[name];
    StopMusicStream(music); // Ensure only one plays at a time
    SetMusicVolume(music, volume);
    PlayMusicStream(music);
}

void SoundManager::Play(const std::string& name) {
    auto it = sounds.find(name);
    if (it == sounds.end()) return;

    if (!IsSoundPlaying(it->second)) {
        PlaySound(it->second);
    }
}

void SoundManager::PlaySoundAtPosition(const std::string& soundName, const Vector3& soundPos, const Vector3& listenerPos, float maxDistance) {
    if (sounds.find(soundName) == sounds.end()) {
        std::cerr << "Sound not found: " << soundName << std::endl;
        return;
    }

    Vector3 delta = Vector3Subtract(soundPos, listenerPos);
    float distance = Vector3Length(delta);
    if (distance > maxDistance) return;

    float volume = Clamp(1.0f - (distance / maxDistance), 0.0f, 1.0f);

    Sound sound = sounds[soundName];
    SetSoundVolume(sound, volume);
    PlaySound(sound);
}

// Update, get functions

void SoundManager::Update() {
    for (auto& [name, music] : musicTracks) {
        UpdateMusicStream(music);
    }
}

Music& SoundManager::GetMusic(const std::string& name) {
    return musicTracks.at(name); 
}

Sound& SoundManager::GetSound(const std::string& name) {
    return sounds.at(name);
}
