#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"

class ResourceManager {
public:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ~ResourceManager();

    static ResourceManager& Get(); // Singleton

    // Load, get functions

    Texture2D& LoadTexture(const std::string& name, const std::string& path);
    Texture2D& GetTexture(const std::string& name);     
    Model& LoadModel(const std::string& name, const std::string& path);
    Model& LoadModelFromMesh(const std::string& name, const Mesh& mesh);
    Model& GetModel(const std::string& name) const;
    Shader& LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);
    Shader& GetShader(const std::string& name) const;
    RenderTexture2D& LoadRenderTexture(const std::string& name, int width, int height);
    RenderTexture2D& GetRenderTexture(const std::string& name) const;
    Font& LoadFont(const std::string& name, const std::string& path);
    Font& GetFont(const std::string& name);
    void LoadAllResources();

    // Unload functions

    void UnloadAll();

    // Shader functions
    
    void UpdateShaders(Camera& camera);
    void SetBloomShaderValues();
    void SetLavaShaderValues();
    void SetLightingShaderValues();
    void SetTerrainShaderValues();
    void SetPortalShaderValues();
    void SetShaderValues();
    void EnsureScreenSizedRTs();
private:

    // Storage maps
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Model> models;
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, RenderTexture2D> renderTextures;
    std::unordered_map<std::string, Font> fonts;

    Texture2D& getFallbackTexture();
    Font& getFallbackFont();
};
