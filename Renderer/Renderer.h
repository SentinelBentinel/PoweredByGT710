#pragma once

#include "SDL3/SDL.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>

#include "../Math/Color.h"
#include "../Graphics/Triangle.h"
#include "../Graphics/Mesh.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"
#include "../Core/Camera.h"
#include "../Graphics/Plane.h"
#include "Stats.h"

enum class RenderMode
{
    Filled,
    Wireframe
};

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();

    float fov = 90.0f;

    bool Initialize();

    bool ProcessEvents();

    void Clear(Color color);

    void DrawPixel(int x, int y, Color color);
    void DrawLine(Vector2 start, Vector2 end, Color color);
    void RasterizeTriangleViewSpace(const Triangle &triangle);
    void RenderMesh(const Mesh &mesh);
    void SetRenderMode(RenderMode mode);
    void DrawText(int x, int y, const std::string &text, Color color);

    Stats &GetStats();
    Camera &GetCamera();

    void Present();

    std::vector<Color> &GetFramebuffer();

private:
    float EdgeFunc(const Vector2 &v1, const Vector2 &v2, const Vector2 &p);
    Vertex TransformVertex(const Vertex &vertex, const Transform &transform);
    Vector2 ProjectVertex(const Vector3 &position);
    bool IsBackFace(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2);
    void PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(const Triangle &triangle, const Plane &plane ,std::vector<Triangle> &output);
    RenderMode renderMode = RenderMode::Filled;
    TTF_Font *font = nullptr;
    Stats stats;
    Camera camera;

    int width;
    int height;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    std::vector<Color> framebufer;
    std::vector<float> depthBuffer;
};