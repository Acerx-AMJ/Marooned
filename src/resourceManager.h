#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"
#include <stdexcept>

class ResourceManager {
public:
    static ResourceManager& Get();
    
    // Texture
    Texture2D& LoadTexture(const std::string& name, const std::string& path);
    //Texture2D& GetTexture(const std::string& name) const;
    Texture2D&  GetTexture(const std::string& name);     
    // Model
    Model&      LoadModel(const std::string& name, const std::string& path);
    Model&      LoadModelFromMesh(const std::string& name, const Mesh& mesh);
    Model&      GetModel(const std::string& name) const;

    // Shader
    Shader&     LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);
    Shader&     GetShader(const std::string& name) const;


    // RenderTexture
    RenderTexture2D& LoadRenderTexture(const std::string& name, int width, int height);
    RenderTexture2D& GetRenderTexture(const std::string& name) const;

    //should these be ResourceManager::LoadAllResources ect..? functions are not part of the class. 
    void LoadAllResources();
    void UpdateShaders(Camera& camera);
    void SetLavaShaderValues();
    void SetLightingShaderValues();
    void SetShaderValues();

    // Clean-up
    void UnloadAll();
    ~ResourceManager();

private:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    template<typename T, typename UnloadFn>
    void UnloadContainer(std::unordered_map<std::string, T>& container, UnloadFn unloadFn) {
        for (auto& kv : container) unloadFn(kv.second);
        container.clear();
    }

    // Storage maps
    std::unordered_map<std::string, Texture2D>      _textures;
    std::unordered_map<std::string, Model>          _models;
    std::unordered_map<std::string, Shader>         _shaders;
    std::unordered_map<std::string, RenderTexture2D> _renderTextures;

    // Fallback texture (not in the map); 'mutable' so const GetTexture can lazy-init it
    mutable Texture2D _fallbackTex{};
    mutable bool      _fallbackReady = false;
    void ensureFallback_() const;   // creates procedural checker if needed

    static ResourceManager* _instance;
};

inline ResourceManager& R = ResourceManager::Get();
