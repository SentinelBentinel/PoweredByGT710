#include "OBJLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

struct OBJIndex
{
    int position;
    int uv;
    int normal;
};

auto ParseIndex = [](const std::string &token)
{
    OBJIndex idx{};

    sscanf(
        token.c_str(),
        "%d/%d/%d",
        &idx.position,
        &idx.uv,
        &idx.normal);

    idx.position--;
    idx.uv--;
    idx.normal--;

    return idx;
};

static Color ColorFromFloats(float r, float g, float b)
{
    return {
        static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f)};
}

void LoadMaterial(const std::filesystem::path &folder, const std::string &materialFile, Mesh &mesh)
{
    std::ifstream file(folder / materialFile);

    if (!file.is_open())
        return;

    std::string line;
    Material *current = nullptr;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "newmtl")
        {
            std::string name;
            ss >> name;
            mesh.materials.emplace_back();
            mesh.materials.back().name = name;
            current = &mesh.materials.back();
        }
        else if (!current)
        {
            continue;
        }
        else if (token == "Ka")
        {
            float r, g, b;
            ss >> r >> g >> b;
            current->ambient = ColorFromFloats(r, g, b);
        }
        else if (token == "Kd")
        {
            float r, g, b;
            ss >> r >> g >> b;
            current->diffuse = ColorFromFloats(r, g, b);
        }
        else if (token == "Ks")
        {
            float r, g, b;
            ss >> r >> g >> b;
            current->specular = ColorFromFloats(r, g, b);
        }
        else if (token == "Ns")
        {
            ss >> current->shininess;
        }
        else if (token == "d" || token == "Tr")
        {
            ss >> current->opacity;
            current->opacity = std::clamp(current->opacity, 0.0f, 1.0f);
        }
        else if (token == "map_Kd")
        {
            std::string textureName;
            ss >> textureName;
            current->diffuseMap.Load((folder / textureName).string());
            current->hasDiffuseMap = true;
        }
    }
};

Mesh OBJLoader::Load(const std::string &path)
{
    Mesh mesh;
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;

    std::filesystem::path objPath(path);
    std::filesystem::path objFolder = objPath.parent_path();

    std::string materialFile;
    int currentMaterialIndex = -1;
    auto FindMaterialIndex = [&](const std::string &name) -> int {
        for (int i = 0; i < static_cast<int>(mesh.materials.size()); ++i)
            if (mesh.materials[i].name == name)
                return i;
        return -1;
    };

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open OBJ: " << path << '\n';
        return mesh;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v")
        {
            Vector3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (type == "vt")
        {
            Vector2 uv;
            ss >> uv.x >> uv.y;
            texcoords.push_back(uv);
        }
        else if (type == "vn")
        {
            Vector3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (type == "f")
        {
            std::vector<std::string> face;
            std::string token;
            while (ss >> token)
                face.push_back(token);

            if (face.size() < 3)
                continue;

            auto MakeVertex = [&](const std::string &s)
            {
                OBJIndex idx = ParseIndex(s);
                Vertex v;
                v.position = positions[idx.position];
                v.uv = texcoords[idx.uv];
                v.normal = normals[idx.normal];
                v.color = {255, 255, 255};

                mesh.indices.push_back(mesh.vertices.size());
                mesh.vertices.push_back(v);
            };

            for (size_t i = 1; i < face.size() - 1; ++i)
            {
                MakeVertex(face[0]);
                MakeVertex(face[i]);
                MakeVertex(face[i + 1]);

                mesh.materialIndices.push_back(currentMaterialIndex);
            }
        }
        else if (type == "mtllib")
        {
            ss >> materialFile;
        }
        else if (type == "usemtl")
        {
            std::string materialName;
            ss >> materialName;
            currentMaterialIndex = FindMaterialIndex(materialName);
        }
    }

    if (!materialFile.empty())
        LoadMaterial(objFolder, materialFile, mesh);

    return mesh;
}
