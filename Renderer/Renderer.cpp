#include "Renderer.h"

#include <algorithm>
#include <iostream>
#include <format>
#include <limits>
#include <cmath>
#include <SDL3_ttf/SDL_ttf.h>
/*
    Poop
*/

static Vertex IntersectNearPlane(const Vertex& inside, const Vertex& outside, float nearPlane)
{
    float t = (nearPlane - inside.position.z) / (outside.position.z - inside.position.z);

    return Vertex::Lerp(inside, outside, t);
}

Renderer::Renderer(int w, int h) : width(w), height(h), framebufer(w * h), depthBuffer(w * h) {};

Renderer::~Renderer()
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (font)
        TTF_CloseFont(font);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
}

Stats &Renderer::GetStats()
{
    return stats;
}

bool Renderer::Initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init err: " << SDL_GetError() << '\n';
        return false;
    }
    if (!TTF_Init())
    {
        std::cerr << "TTF_Init err: " << SDL_GetError() << '\n';
    }
    if (!SDL_CreateWindowAndRenderer("PR3TEST", width, height, 0, &window, &renderer))
    {
        std::cerr << "Window/Renderer err: " << SDL_GetError() << '\n';
        return false;
    }

    font = TTF_OpenFont("../Assets/JetBrainsMono-Bold.ttf", 16);

    if (!font)
    {
        std::cerr << "Font err: " << SDL_GetError() << '\n';
        return false;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height);

    if (!texture)
    {
        std::cerr << "Texture err: " << SDL_GetError() << '\n';
        return false;
    }

    return true;
}

bool Renderer::ProcessEvents()
{
    SDL_Event event;
    const bool *keys = SDL_GetKeyboardState(nullptr);
    Camera &cam = GetCamera();

    float speed = 4.0f;

    if (keys[SDL_SCANCODE_W])
        cam.position.z += speed;
    if (keys[SDL_SCANCODE_S])
        cam.position.z -= speed;
    if (keys[SDL_SCANCODE_A])
        cam.position.x -= speed;
    if (keys[SDL_SCANCODE_D])
        cam.position.x += speed;
    if (keys[SDL_SCANCODE_SPACE])
        cam.position.y += speed;
    if (keys[SDL_SCANCODE_LCTRL])
        cam.position.y -= speed;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            return false;
    }
    return true;
}

void Renderer::Clear(Color color)
{
    std::fill(
        framebufer.begin(),
        framebufer.end(),
        color);
    std::fill(
        depthBuffer.begin(),
        depthBuffer.end(),
        std::numeric_limits<float>::infinity());

    stats.vertices = 0;
    stats.triangles = 0;
    stats.trianglesCulled = 0;
    stats.meshes = 0;
    stats.pixelsDrawn = 0;
}

void Renderer::Present()
{
    SDL_UpdateTexture(
        texture,
        nullptr,
        framebufer.data(),
        width * sizeof(Color));

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);

    Stats &s = GetStats();

    DrawText(10, 30,
             std::format("FPS: {:.0f}", s.fps),
             {255, 255, 255});

    DrawText(10, 50,
             std::format("Frame: {:.2f} ms", s.frameTime),
             {255, 255, 255});

    DrawText(10, 70,
             std::format("Meshes: {}", s.meshes),
             {255, 255, 255});

    DrawText(10, 90,
             std::format("Triangles: {}", s.triangles),
             {255, 255, 255});

    DrawText(10, 110,
             std::format("Vertices: {}", s.vertices),
             {255, 255, 255});

    DrawText(10, 130,
             std::format("Pixels: {}", s.pixelsDrawn),
             {255, 255, 255});

    DrawText(10, 150,
             std::format("Culled: {}", s.trianglesCulled),
             {255, 255, 255});

    SDL_RenderPresent(renderer);
}

std::vector<Color> &Renderer::GetFramebuffer()
{
    return framebufer;
}

void Renderer::SetRenderMode(RenderMode mode)
{
    renderMode = mode;
}

void Renderer::DrawPixel(int x, int y, Color color)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;
    framebufer[y * width + x] = color;
}

void Renderer::DrawLine(Vector2 start, Vector2 end, Color color)
{
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

    if (steps == 0)
    {
        DrawPixel(
            static_cast<int>(start.x),
            static_cast<int>(start.y),
            color);
        return;
    }

    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    float x = start.x;
    float y = start.y;

    for (int i = 0; i <= steps; i++)
    {
        DrawPixel(
            static_cast<int>(std::round(x)),
            static_cast<int>(std::round(y)),
            color);

        x += xIncrement;
        y += yIncrement;
    }
}

void Renderer::DrawText(int x, int y, const std::string &text, Color color)
{
    SDL_Color sdlColor{
        color.r,
        color.g,
        color.b,
        255};

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, sdlColor);

    if (!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FRect dst{
        (float)x,
        (float)y,
        (float)surface->w,
        (float)surface->h};

    SDL_RenderTexture(renderer, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

Camera &Renderer::GetCamera()
{
    return camera;
}

Vector2 Renderer::ProjectVertex(const Vector3 &position)
{
    static Matrix4 projection = Matrix4::Perspective(
        fov * (3.1415926535f / 180.0f),
        static_cast<float>(width) / height,
        0.1f,
        1000.0f);

    Vector4 clip = projection.MultiplyPoint4(position);

    if (clip.w <= 0.0f)
        return {-1000000.0f, -1000000.0f};

    float invW = 1.0f / clip.w;

    float ndcX = clip.x * invW;
    float ndcY = clip.y * invW;

    return {
        (ndcX + 1.0f) * 0.5f * width,
        (1.0f - ndcY) * 0.5f * height};
}

bool Renderer::IsBackFace(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    Vector3 edge1 = v1.position - v0.position;
    Vector3 edge2 = v2.position - v0.position;

    Vector3 normal = Vector3::Cross(edge1, edge2);

    Vector3 centroid = (v0.position + v1.position + v2.position) * (1.0f / 3.0f);
    Vector3 viewDir = Vector3{-centroid.x, -centroid.y, -centroid.z};

    return Vector3::Dot(normal, viewDir) <= 0.0f;
}

void Renderer::RasterizeTriangle(const Triangle &triangle)
{
    Vertex v0 = triangle.v0;
    Vertex v1 = triangle.v1;
    Vertex v2 = triangle.v2;

    Vector2 p0 = ProjectVertex(v0.position);
    Vector2 p1 = ProjectVertex(v1.position);
    Vector2 p2 = ProjectVertex(v2.position);

    float z0 = v0.position.z;
    float z1 = v1.position.z;
    float z2 = v2.position.z;

    float area = EdgeFunc(
        p0,
        p1,
        p2);

    if (std::abs(area) < 0.0001f)
        return;

    int minX = std::max(0, static_cast<int>(std::floor(std::min({p0.x, p1.x, p2.x}))));
    int maxX = std::min(width - 1, static_cast<int>(std::ceil(std::max({p0.x, p1.x, p2.x}))));
    int minY = std::max(0, static_cast<int>(std::floor(std::min({p0.y, p1.y, p2.y}))));
    int maxY = std::min(height - 1, static_cast<int>(std::ceil(std::max({p0.y, p1.y, p2.y}))));

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 p = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};
            float w0 = EdgeFunc(p1, p2, p);
            float w1 = EdgeFunc(p2, p0, p);
            float w2 = EdgeFunc(p0, p1, p);

            if ((area >= 0 && w0 >= 0 && w1 >= 0 && w2 >= 0) || (area < 0 && w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                w0 /= area;
                w1 /= area;
                w2 /= area;

                float depth = w0 * z0 + w1 * z1 + w2 * z2;

                Color color;

                color.r = static_cast<unsigned char>(w0 * v0.color.r + w1 * v1.color.r + w2 * v2.color.r);
                color.g = static_cast<unsigned char>(w0 * v0.color.g + w1 * v1.color.g + w2 * v2.color.g);
                color.b = static_cast<unsigned char>(w0 * v0.color.b + w1 * v1.color.b + w2 * v2.color.b);

                int index = y * width + x;

                if (depth < depthBuffer[index])
                {
                    depthBuffer[index] = depth;

                    framebufer[index] = color;

                    stats.pixelsDrawn++;
                }
            }
        }
    }
}

void Renderer::RenderMesh(const Mesh &mesh)
{
    stats.meshes++;
    for (size_t i = 0; i < mesh.indices.size(); i += 3)
    {
        stats.triangles++;
        stats.vertices += 3;
        
        Triangle triangle;

        triangle.v0 = TransformVertex(mesh.vertices[mesh.indices[i]], mesh.transform);
        triangle.v1 = TransformVertex(mesh.vertices[mesh.indices[i + 1]], mesh.transform);
        triangle.v2 = TransformVertex(mesh.vertices[mesh.indices[i + 2]], mesh.transform);

        if (IsBackFace(triangle.v0, triangle.v1, triangle.v2))
        {
            stats.trianglesCulled++;
            continue;
        }

        std::vector<Triangle> clipped;
        PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(triangle, clipped);
        
        for (const Triangle& t : clipped)
        {
            if (renderMode == RenderMode::Filled)
            {
                RasterizeTriangle(t);
            } else
            {
                Vector2 p0 = ProjectVertex(t.v0.position);
                Vector2 p1 = ProjectVertex(t.v1.position);
                Vector2 p2 = ProjectVertex(t.v2.position);

                DrawLine(p0, p1, t.v0.color);
                DrawLine(p1, p2, t.v1.color);
                DrawLine(p2, p0, t.v2.color);
            }
        }
    }
}

float Renderer::EdgeFunc(const Vector2 &v1, const Vector2 &v2, const Vector2 &p)
{
    return (p.x - v1.x) * (v2.y - v1.y) - (p.y - v1.y) * (v2.x - v1.x);
}

Vertex Renderer::TransformVertex(const Vertex &vertex, const Transform &transform)
{
    Vertex result = vertex;

    Matrix4 model =
        Matrix4::Translation(transform.position) *
        Matrix4::RotationZ(transform.rotation.z) *
        Matrix4::RotationY(transform.rotation.y) *
        Matrix4::RotationX(transform.rotation.x) *
        Matrix4::Scale(transform.scale);

    Vector3 world = model.MultiplyPoint(vertex.position);

    Vector3 view = camera.GetViewMatrix().MultiplyPoint(world);

    result.position = view;

    return result;
}

void Renderer::PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(const Triangle& triangle, std::vector<Triangle>& output)
{
    constexpr float nearPlane = 0.1f;

    Vertex verts[3] =
    {
        TransformVertex(triangle.v0, triangle.transform),
        TransformVertex(triangle.v1, triangle.transform),
        TransformVertex(triangle.v2, triangle.transform)
    };

    Vertex inside[3];
    Vertex outside[3];

    int insideCount = 0;
    int outsideCount = 0;

    for (int i = 0; i < 3; i++)
    {
        if (verts[i].position.z >= nearPlane)
            inside[insideCount++] = verts[i];
        else
            outside[outsideCount++] = verts[i];
    }

    if (insideCount == 0)
        return;

    if (insideCount == 3)
    {
        Triangle t;
        t.v0 = inside[0];
        t.v1 = inside[1];
        t.v2 = inside[2];

        output.push_back(t);
        return;
    }

    if (insideCount == 1)
    {
        Vertex a = inside[0];

        Vertex b = IntersectNearPlane(a, outside[0], nearPlane);
        Vertex c = IntersectNearPlane(a, outside[1], nearPlane);

        Triangle t;

        t.v0 = a;
        t.v1 = b;
        t.v2 = c;

        output.push_back(t);

        return;
    }

    if (insideCount == 2)
    {
        Vertex a = inside[0];
        Vertex b = inside[1];

        Vertex c = IntersectNearPlane(a, outside[0], nearPlane);
        Vertex d = IntersectNearPlane(b, outside[0], nearPlane);

        Triangle t1;
        t1.v0 = a;
        t1.v1 = b;
        t1.v2 = c;

        Triangle t2;
        t2.v0 = b;
        t2.v1 = d;
        t2.v2 = c;

        output.push_back(t1);
        output.push_back(t2);

        return;
    }
}