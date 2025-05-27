#include "raylib.h"
#include <vector>
#include <raymath.h>

struct TerrainChunk {
    Model model;
    Vector3 position;
    BoundingBox bounds;
};

std::vector<TerrainChunk> LoadTerrainChunks(Image fullHeightmap, int tiles, Vector3 fullScale, Shader shader) {
    std::vector<TerrainChunk> chunks;
    int tileSize = fullHeightmap.width / tiles;

    for (int z = 0; z < tiles; z++) {
        for (int x = 0; x < tiles; x++) {
            Rectangle crop = { (float)(x * tileSize), (float)(z * tileSize), (float)tileSize, (float)tileSize };
            Image tileImg = ImageCopy(fullHeightmap);
            ImageCrop(&tileImg, crop);

            Vector3 scale = {
                fullScale.x / tiles,
                fullScale.y,
                fullScale.z / tiles
            };

            Mesh mesh = GenMeshHeightmap(tileImg, scale);
            UploadMesh(&mesh, true);
            UnloadImage(tileImg);

            Model model = LoadModelFromMesh(mesh);
            model.materials[0].shader = shader;

            Vector3 pos = {
                -fullScale.x / 2 + x * scale.x,
                0.0f,
                -fullScale.z / 2 + z * scale.z
            };

            BoundingBox bounds = GetMeshBoundingBox(mesh);
            bounds.min = Vector3Add(bounds.min, pos);
            bounds.max = Vector3Add(bounds.max, pos);

            chunks.push_back({ model, pos, bounds });
        }
    }

    return chunks;
}

void DrawTerrainChunks(const std::vector<TerrainChunk>& chunks) {
    for (const auto& chunk : chunks) {
        DrawModel(chunk.model, chunk.position, 1.0f, WHITE);
    }
}
