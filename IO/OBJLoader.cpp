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