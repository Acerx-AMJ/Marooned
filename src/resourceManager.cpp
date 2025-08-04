// ResourceManager.cpp
#include "ResourceManager.h"


Texture2D& ResourceManager::LoadTexture(const char* path) {
    Texture2D tex = ::LoadTexture(path);
    _textures.push_back(tex);
    return _textures.back();
}


Model& ResourceManager::LoadModel(const char* path) {
    Model m = ::LoadModel(path);
    _models.push_back(m);
    return _models.back();
}

Shader& ResourceManager::LoadShader(const char* vs, const char* fs) {
    Shader s = ::LoadShader(vs, fs);
    _shaders.push_back(s);
    return _shaders.back();
}


void ResourceManager::UnloadAll() {
    _UnloadContainer(_textures, ::UnloadTexture);
    _UnloadContainer(_models,   ::UnloadModel);
    _UnloadContainer(_shaders,  ::UnloadShader);

}
