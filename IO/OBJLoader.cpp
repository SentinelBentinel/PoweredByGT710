#include "OBJLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>

struct OBJIndex
{
    int position;
    int uv;
    int normal;
};

auto ParseIndex = [](const std::string& token)
{
    OBJIndex idx{};

    sscanf(
        token.c_str(),
        "%d/%d/%d",
        &idx.position,
        &idx.uv,
        &idx.normal
    );

    idx.position--;
    idx.uv--;
    idx.normal--;

    return idx;
};

Mesh OBJLoader::Load(const std::string &path)
{
    Mesh mesh;

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;

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

        // Vertex yippie

        if (type == "v")
        {
            Vector3 pos;
            ss >> pos.x >> pos.y >> pos.z;

            positions.push_back(pos);
        }

        // Face

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
                OBJIndex i = ParseIndex(s);

                Vertex v;
                v.position = positions[i.position];
                v.uv = texcoords[i.uv];
                v.normal = normals[i.normal];
                v.color = {255,255,255};

                mesh.indices.push_back(mesh.vertices.size());
                mesh.vertices.push_back(v);
            };

            for (size_t i = 1; i < face.size() - 1; i++)
            {
                MakeVertex(face[0]);
                MakeVertex(face[i]);
                MakeVertex(face[i + 1]);
            }
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

        else
        {
            continue;
        }
    }

    mesh.transform.position = {0,0,0};
    mesh.transform.rotation = {0,0,0};
    mesh.transform.scale = {1,1,1};

    std::cout << "Loaded OBJ\n";
    std::cout << "Vertices: " << mesh.vertices.size() << '\n';
    std::cout << "Triangles: " << mesh.indices.size() / 3 << '\n';

    return mesh;
}
