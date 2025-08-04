// ResourceManager.h
#pragma once
#include <vector>
#include <unordered_map>
#include <functional>
#include "raylib.h"
#include <string>

class ResourceManager {
public:
    // Load
    Texture2D& LoadTexture(const char* path);
    Model&     LoadModel(const char* path);
    Shader&    LoadShader(const char* vsPath, const char* fsPath);


    // Unload everything
    void UnloadAll();

private:
    std::vector<Texture2D>                     _textures;
    std::vector<Model>                         _models;
    std::vector<Shader>                        _shaders;

   
    // now unloadFn can be any void(T)
    template<typename T>
    void _UnloadContainer(std::vector<T>& container,
                        void(*unloadFn)(T))
    {
        for (auto& item : container) unloadFn(item);
        container.clear();
    }

};
