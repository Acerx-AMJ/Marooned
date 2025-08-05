
#include "ResourceManager.h"

ResourceManager* ResourceManager::_instance = nullptr;

ResourceManager& ResourceManager::Get() {
    if (!_instance) _instance = new ResourceManager();
    return *_instance;
}

// Texture
Texture2D& ResourceManager::LoadTexture(const std::string& name, const std::string& path) {
    auto it = _textures.find(name);
    if (it != _textures.end()) return it->second;
    Texture2D tex = ::LoadTexture(path.c_str());
    _textures.emplace(name, tex);
    return _textures[name];
}
Texture2D& ResourceManager::GetTexture(const std::string& name) const {
    auto it = _textures.find(name);
    if (it == _textures.end()) throw std::runtime_error("Texture not found: " + name);
    return const_cast<Texture2D&>(it->second);
}

// Model
Model& ResourceManager::LoadModel(const std::string& name, const std::string& path) {
    auto it = _models.find(name);
    if (it != _models.end()) return it->second;
    Model m = ::LoadModel(path.c_str());
    _models.emplace(name, m);
    return _models[name];
}
Model& ResourceManager::LoadModelFromMesh(const std::string& name, const Mesh& mesh) {
    auto it = _models.find(name);
    if (it != _models.end()) return it->second;
    Model m = ::LoadModelFromMesh(mesh);
    _models.emplace(name, m);
    return _models[name];
}
Model& ResourceManager::GetModel(const std::string& name) const {
    auto it = _models.find(name);
    if (it == _models.end()) throw std::runtime_error("Model not found: " + name);
    return const_cast<Model&>(it->second);
}

// Shader
Shader& ResourceManager::LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
    auto it = _shaders.find(name);
    if (it != _shaders.end()) return it->second;
    Shader s = ::LoadShader(vsPath.c_str(), fsPath.c_str());
    _shaders.emplace(name, s);
    return _shaders[name];
}
Shader& ResourceManager::GetShader(const std::string& name) const {
    auto it = _shaders.find(name);
    if (it == _shaders.end()) throw std::runtime_error("Shader not found: " + name);
    return const_cast<Shader&>(it->second);
}


// RenderTexture
RenderTexture2D& ResourceManager::LoadRenderTexture(const std::string& name, int w, int h) {
    auto it = _renderTextures.find(name);
    if (it != _renderTextures.end()) return it->second;
    RenderTexture2D rt = ::LoadRenderTexture(w, h);
    _renderTextures.emplace(name, rt);
    return _renderTextures[name];
}
RenderTexture2D& ResourceManager::GetRenderTexture(const std::string& name) const {
    auto it = _renderTextures.find(name);
    if (it == _renderTextures.end()) throw std::runtime_error("RenderTexture not found: " + name);
    return const_cast<RenderTexture2D&>(it->second);
}


void ResourceManager::UnloadAll() {
    UnloadContainer(_textures,        ::UnloadTexture);
    UnloadContainer(_models,          ::UnloadModel);
    UnloadContainer(_shaders,         ::UnloadShader);
    UnloadContainer(_renderTextures,  ::UnloadRenderTexture);
}

ResourceManager::~ResourceManager() {
    UnloadAll();
}

