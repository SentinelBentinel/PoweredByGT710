#include "Renderer/Renderer.h"
#include "Graphics/Triangle.h"
#include "Graphics/Mesh.h"
#include "Core/Timer.h"
#include "IO/OBJLoader.h"
#include "Core/Scene.h"

#include <vector>
#include <iostream>
#include <filesystem>

std::vector<Mesh> meshes;

void Update(const Timer &timer, Scene &scene)
{
    (void)timer;
    for (Mesh &mesh : scene.GetMeshes())
    {
        mesh.transform.rotation.z += 0.01f;
    }
}

void Render(Renderer &renderer, Scene &scene)
{
    renderer.Clear({30, 30, 30});

    for (const Mesh &mesh : scene.GetMeshes())
    {
        renderer.RenderMesh(mesh);
    }

    renderer.RasterizeVisibleTriangles();
    renderer.Present();
}

int main()
{
    std::cout << std::filesystem::current_path() << '\n';
    Renderer renderer(640,480);
    Scene scene;
    Timer timer;

    if (!renderer.Initialize())
        return -1;

    renderer.SetRenderMode(RenderMode::Filled);
    renderer.GetCamera().position.z = -200;

    scene.LoadMesh("../assets/teaofpotsLOW.obj")
        .transform.position.x = 10.0f;
    scene.LoadMesh("../assets/helmet.obj");

    while (renderer.ProcessEvents())
    {
        timer.Update();
        renderer.GetStats().fps = timer.GetFPS();
        renderer.GetStats().frameTime = timer.GetFrameTime();
        Update(timer, scene);
        Render(renderer, scene);
    }

    return 0;
}