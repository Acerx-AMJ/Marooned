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

void SoundManager::LoadMusic(const std::string& name, const std::string& filePath) {
    Music music = LoadMusicStream(filePath.c_str());
    musicTracks[name] = music;
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

void SoundManager::Update() {
    for (auto& [name, music] : musicTracks) {
        UpdateMusicStream(music);
    }
}

Music& SoundManager::GetMusic(const std::string& name) {
    return musicTracks[name]; 
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
    for (auto& [name, music] : musicTracks){
        ::UnloadMusicStream(music);
    }
    musicTracks.clear();

}

void SoundManager::LoadSounds() {
    //Sounds
    SoundManager::GetInstance().LoadSound("dinoHit", "assets/sounds/dinoHit.ogg");
    SoundManager::GetInstance().LoadSound("dinoDeath", "assets/sounds/dinoDeath.ogg");
    SoundManager::GetInstance().LoadSound("dinoTweet", "assets/sounds/dino1.ogg");
    SoundManager::GetInstance().LoadSound("dinoTarget", "assets/sounds/dino2.ogg");
    SoundManager::GetInstance().LoadSound("dinoTweet2", "assets/sounds/dino3.ogg");
    SoundManager::GetInstance().LoadSound("dinoBite", "assets/sounds/bite.ogg");
    SoundManager::GetInstance().LoadSound("reload", "assets/sounds/reload.ogg");
    SoundManager::GetInstance().LoadSound("shotgun", "assets/sounds/shotgun.ogg");

    SoundManager::GetInstance().LoadSound("step1", "assets/sounds/step1.ogg");
    SoundManager::GetInstance().LoadSound("step2", "assets/sounds/step2.ogg");
    SoundManager::GetInstance().LoadSound("step3", "assets/sounds/step3.ogg");
    SoundManager::GetInstance().LoadSound("step4", "assets/sounds/step4.ogg");

    SoundManager::GetInstance().LoadSound("phit1", "assets/sounds/PlayerHit1.ogg");
    SoundManager::GetInstance().LoadSound("phit2", "assets/sounds/PlayerHit2.ogg");

    SoundManager::GetInstance().LoadSound("doorOpen", "assets/sounds/doorOpen.ogg");
    SoundManager::GetInstance().LoadSound("doorClose", "assets/sounds/doorCLose.ogg");
    SoundManager::GetInstance().LoadSound("swipe1", "assets/sounds/swipe1.ogg");
    SoundManager::GetInstance().LoadSound("swipe2", "assets/sounds/swipe2.ogg");
    SoundManager::GetInstance().LoadSound("swipe3", "assets/sounds/swipe3.ogg");
    SoundManager::GetInstance().LoadSound("swordHit", "assets/sounds/swordHit.ogg");
    SoundManager::GetInstance().LoadSound("swordBlock", "assets/sounds/swordBlock.ogg");
    SoundManager::GetInstance().LoadSound("swordBlock2", "assets/sounds/swordBlock2.ogg");
    SoundManager::GetInstance().LoadSound("bones", "assets/sounds/bones.ogg");
    SoundManager::GetInstance().LoadSound("bones2", "assets/sounds/bones2.ogg");
    SoundManager::GetInstance().LoadSound("gulp", "assets/sounds/gulp.ogg");
    SoundManager::GetInstance().LoadSound("clink", "assets/sounds/clink.ogg");
    SoundManager::GetInstance().LoadSound("lockedDoor", "assets/sounds/lockedDoor.ogg");
    SoundManager::GetInstance().LoadSound("unlock", "assets/sounds/unlock.ogg");
    SoundManager::GetInstance().LoadSound("key", "assets/sounds/KeyGet.ogg");
    SoundManager::GetInstance().LoadSound("barrelBreak", "assets/sounds/barrelBreak.ogg");
    SoundManager::GetInstance().LoadSound("musket", "assets/sounds/musket.ogg");
    SoundManager::GetInstance().LoadSound("chestOpen", "assets/sounds/chestOpen.ogg");
    SoundManager::GetInstance().LoadSound("spiderBite1", "assets/sounds/spiderBite1.ogg");
    SoundManager::GetInstance().LoadSound("spiderBite2", "assets/sounds/spiderBite2.ogg");
    SoundManager::GetInstance().LoadSound("spiderDeath", "assets/sounds/spiderDeath.ogg");
    SoundManager::GetInstance().LoadSound("flame1", "assets/sounds/flame1.ogg");
    SoundManager::GetInstance().LoadSound("flame2", "assets/sounds/flame2.ogg");
    SoundManager::GetInstance().LoadSound("explosion", "assets/sounds/explosion.ogg");
    SoundManager::GetInstance().LoadSound("staffHit", "assets/sounds/staffHit.ogg");
    SoundManager::GetInstance().LoadSound("iceMagic", "assets/sounds/iceMagic.ogg");
    
    //music (ambience)
    SoundManager::GetInstance().LoadMusic("dungeonAir", "assets/sounds/dungeonAir.ogg");
    SoundManager::GetInstance().LoadMusic("jungleAmbience", "assets/sounds/jungleSounds.ogg");
}
