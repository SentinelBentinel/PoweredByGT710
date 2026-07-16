#include "Renderer/Renderer.h"
#include "Graphics/Triangle.h"
#include "Graphics/Mesh.h"
#include "Core/Timer.h"
#include "IO/OBJLoader.h"

#include <vector>
#include <iostream>
#include <filesystem>

std::vector<Mesh> meshes;

void Update(const Timer &timer)
{
    (void)timer;
    meshes[0].transform.rotation.x += 0.01f;
    meshes[0].transform.rotation.y -= 0.01f;
}

void Render(Renderer &renderer)
{
    renderer.Clear({30, 30, 30});

    for (const Mesh &mesh : meshes)
    {
        renderer.RenderMesh(mesh);
    }

    renderer.Present();
}

int main()
{
    std::cout << std::filesystem::current_path() << '\n';
    Renderer renderer(800, 600);
    Timer timer;

    if (!renderer.Initialize())
        return -1;

    renderer.SetRenderMode(RenderMode::Filled);
    renderer.GetCamera().position.z = -200;

    // Mesh teapotLQ = OBJLoader::Load("../assets/teaofpotsLOW.obj");
    // teapotLQ.transform.position = {10,0,0};
    // Mesh Crate = OBJLoader::Load("../assets/Crate.obj");

    // meshes.push_back(teapotLQ);
    // meshes.push_back(Crate);

    Mesh cube = OBJLoader::Load("../assets/Cubus.obj");
    meshes.push_back(cube);

    while (renderer.ProcessEvents())
    {
        timer.Update();
        renderer.GetStats().fps = timer.GetFPS();
        renderer.GetStats().frameTime = timer.GetFrameTime();
        Update(timer);
        Render(renderer);
    }

    return 0;
}