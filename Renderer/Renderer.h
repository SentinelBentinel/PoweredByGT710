#pragma once

#include "SDL3/SDL.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

#include "../Math/Color.h"
#include "../Graphics/Triangle.h"
#include "../Graphics/Mesh.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"
#include "../Core/Camera.h"
#include "../Graphics/Plane.h"
#include "../Graphics/Light.h"
#include "../Graphics/Texture.h"
#include "Stats.h"

enum class RenderMode
{
    Filled,
    Wireframe
};

struct VisibleTriangle
{
    Triangle triangle;
    const Mesh *mesh = nullptr;
    int materialIndex = -1;
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
    void RasterizeTriangleViewSpace(const Triangle &triangle, const Mesh &mesh);
    void WriteFragment(int x, int y, float depth, const Color &color);
    void RenderMesh(const Mesh &mesh);
    void RasterizeVisibleTriangles();
    void SetRenderMode(RenderMode mode);
    void SetUseMultithreading(bool enabled);
    void SetUseTiles(bool enabled);
    void SetUseSIMD(bool enabled);
    void DrawText(int x, int y, const std::string &text, Color color);

    Stats &GetStats();
    Camera &GetCamera();

    void Present();

    std::vector<Color> &GetFramebuffer();

private:
    float EdgeFunc(const Vector2 &v1, const Vector2 &v2, const Vector2 &p);
    Vertex TransformVertex(const Vertex &vertex, const Transform &transform);
    Vector2 ProjectVertex(Vertex &vertex);
    bool IsBackFace(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2);
    void PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(const Triangle &triangle, const Plane &plane, std::vector<Triangle> &output);
    Vector3 ComputeFaceNormal(const Vertex &v0, const Vertex &v1, const Vertex &v2);
    void GeometryStage(const Mesh &mesh);
    void WriteFragmentUnlocked(int x, int y, float depth, const Color &color);
    int RasterizeTriangleViewSpace(const Triangle &triangle, const Mesh &mesh, int minX, int maxX, int minY, int maxY, bool useLock = true, int materialIndex = -1);
    Vertex TransformSkyboxVertex(const Vertex &vertex, const Transform &transform);

    RenderMode renderMode = RenderMode::Filled;
    bool useMultithreading = true;
    bool useTiles = true;
    bool useSIMD = true;
    TTF_Font *font = nullptr;
    Stats stats;
    Camera camera;
    DirectionalLight light;

    int width;
    int height;
    static constexpr int TileSize = 16;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *RenderTexture = nullptr;

    std::vector<Color> framebufer;
    std::vector<float> depthBuffer;
    std::vector<VisibleTriangle> visibleTriangles;
    std::mutex pixelMutex;
};