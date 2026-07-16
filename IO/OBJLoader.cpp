#include "OBJLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>

Mesh OBJLoader::Load(const std::string &path)
{
    Mesh mesh;

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
            float x,y,z;
            ss >> x >> y >> z;

            Vertex vertex;
            vertex.position = {x,y,z};

            vertex.color = {25,255,144};

            mesh.vertices.push_back(vertex);
        }

        // Face

        else if (type == "f")
        {
            std::string a,b,c;
            ss >> a >> b >> c;

            auto ParseIndex = [](const std::string &token)
            {
                std::stringstream parser(token);

                std::string index;

                std::getline(parser, index, '/');

                return std::stoi(index) - 1;
            };

            mesh.indices.push_back(ParseIndex(a));
            mesh.indices.push_back(ParseIndex(b));
            mesh.indices.push_back(ParseIndex(c));
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

void ComputeVertexNormals(Mesh &mesh)
{
    for (Vertex &v : mesh.vertices)
        v.normal = {0,0,0};

    for (size_t i = 0; i < mesh.indices.size(); i += 3)
    {
        Vertex &v0 = mesh.vertices[mesh.indices[i]];
        Vertex &v1 = mesh.vertices[mesh.indices[i + 1]];
        Vertex &v2 = mesh.vertices[mesh.indices[i + 2]];

        Vector3 edge1 = v1.position - v0.position;
        Vector3 edge2 = v2.position - v0.position;

        Vector3 normal = Vector3::Cross(edge1, edge2).Normalized();

        v0.normal += normal;
        v1.normal += normal;
        v2.normal += normal;
    }

    for (Vertex &v : mesh.vertices)
        v.normal = v.normal.Normalized();
}