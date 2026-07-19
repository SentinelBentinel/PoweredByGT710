#pragma once

#include "../Graphics/Mesh.h"
#include "../IO/OBJLoader.h"

class Scene
{
public:
    Mesh &LoadMesh(const std::string &path)
    {
        meshes.push_back(OBJLoader::Load(path));
        return meshes.back();
    }

    std::vector<Mesh> &GetMeshes()
    {
        return meshes;
    }

private:
    std::vector<Mesh> meshes;
};