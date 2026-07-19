#include "Renderer.h"

#include <algorithm>
#include <iostream>
#include <format>
#include <limits>
#include <cmath>
#include <thread>
#include <SDL3_ttf/SDL_ttf.h>
#if defined(__SSE__) || defined(__AVX__)
#include <immintrin.h>
#endif
/*
    Poop
*/

float DistanceToPlane(const Plane &plane, const Vector3 &point)
{
    return Vector3::Dot(plane.normal, point) + plane.distance;
}

static Vertex IntersectPlane(const Vertex &inside, const Vertex &outside, const Plane &plane)
{
    float da = DistanceToPlane(plane, inside.position);
    float db = DistanceToPlane(plane, outside.position);
    float t = da / (da - db);

    return Vertex::Lerp(inside, outside, t);
}

Plane NormalizePlane(const Plane &plane)
{
    float length = plane.normal.Length();

    return {
        plane.normal / length,
        plane.distance / length};
}

Renderer::Renderer(int w, int h) : width(w), height(h), framebufer(w * h), depthBuffer(w * h) {};

Renderer::~Renderer()
{
    if (RenderTexture)
        SDL_DestroyTexture(RenderTexture);
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

    RenderTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height);

    if (!RenderTexture)
    {
        std::cerr << "Texture err: " << SDL_GetError() << '\n';
        return false;
    }

    light.direction = light.direction.Normalized();

    stats.hardwareThreads = static_cast<int>(std::thread::hardware_concurrency());
    stats.logicalCores = stats.hardwareThreads > 0 ? stats.hardwareThreads : 1;

    SDL_SetWindowRelativeMouseMode(window, true);

    return true;
}

bool Renderer::ProcessEvents()
{
    SDL_Event event;
    const bool *keys = SDL_GetKeyboardState(nullptr);
    Camera &cam = GetCamera();

    float speed = 1.0f;
    float sensitivity = 0.003f;

    Vector3 forward = cam.Forward();
    Vector3 right = cam.Right();
    Vector3 up = Vector3::Cross(right, forward).Normalized();

    if (keys[SDL_SCANCODE_W])
        cam.position += forward * speed;
    if (keys[SDL_SCANCODE_S])
        cam.position -= forward * speed;
    if (keys[SDL_SCANCODE_A])
        cam.position -= right * speed;
    if (keys[SDL_SCANCODE_D])
        cam.position += right * speed;
    if (keys[SDL_SCANCODE_SPACE])
        cam.position -= up * speed;
    if (keys[SDL_SCANCODE_LCTRL])
        cam.position += up * speed;
    if (keys[SDL_SCANCODE_ESCAPE])
        return false;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            return false;
        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            cam.rotation.y += event.motion.xrel * sensitivity;
            cam.rotation.x += event.motion.yrel * sensitivity;
        }
    }

    cam.rotation.x = std::clamp(cam.rotation.x, -1.55f, 1.55f);
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
    stats.pixelsDrawn.store(0);
    stats.activeThreads = 1;
    visibleTriangles.clear();
}

void Renderer::Present()
{
    SDL_UpdateTexture(
        RenderTexture,
        nullptr,
        framebufer.data(),
        width * sizeof(Color));

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, RenderTexture, nullptr, nullptr);

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
             std::format("Pixels: {}", static_cast<int>(s.pixelsDrawn.load())),
             {255, 255, 255});

    DrawText(10, 150,
             std::format("Culled: {}", s.trianglesCulled),
             {255, 255, 255});

    DrawText(10, 170,
             std::format("Threads: {}/{}", s.activeThreads, s.logicalCores),
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

void Renderer::SetUseMultithreading(bool enabled)
{
    useMultithreading = enabled;
}

void Renderer::SetUseTiles(bool enabled)
{
    useTiles = enabled;
}

void Renderer::SetUseSIMD(bool enabled)
{
    useSIMD = enabled;
}

Vector3 Renderer::ComputeFaceNormal(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    Vector3 edge1 = v1.position - v0.position;
    Vector3 edge2 = v2.position - v0.position;

    return Vector3::Cross(edge1, edge2).Normalized();
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

Vector2 Renderer::ProjectVertex(Vertex &vertex)
{
    static Matrix4 projection = Matrix4::Perspective(
        fov * (3.1415926535f / 180.0f),
        static_cast<float>(width) / height,
        0.1f,
        1000.0f);

    Vector4 clip = projection.MultiplyPoint4(vertex.position);

    if (clip.w <= 0.0f)
        return {-1000000.0f, -1000000.0f};

    float invW = 1.0f / clip.w;

    vertex.invW = invW;
    vertex.depth = clip.z * vertex.invW;

    float ndcX = clip.x * invW;
    float ndcY = clip.y * invW;

    return {
        (ndcX + 1.0f) * 0.5f * width,
        (1.0f - ndcY) * 0.5f * height};
}

bool Renderer::IsBackFace(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2)
{
    float area = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);

    return area <= 0.0f;
}

void Renderer::RasterizeTriangleViewSpace(const Triangle &triangle, const Mesh &mesh)
{
    RasterizeTriangleViewSpace(triangle, mesh, 0, width - 1, 0, height - 1, true, -1);
}

int Renderer::RasterizeTriangleViewSpace(const Triangle &triangle, const Mesh &mesh, int tileMinX, int tileMaxX, int tileMinY, int tileMaxY, bool useLock, int materialIndex)
{
    Vertex v0 = triangle.v0;
    Vertex v1 = triangle.v1;
    Vertex v2 = triangle.v2;

    Vector2 p0 = ProjectVertex(v0);
    Vector2 p1 = ProjectVertex(v1);
    Vector2 p2 = ProjectVertex(v2);

    float area = EdgeFunc(p0, p1, p2);

    if (std::abs(area) < 0.0001f)
        return 0;

    const int minX = std::max(0, static_cast<int>(std::floor(std::min({p0.x, p1.x, p2.x}))));
    const int maxX = std::min(width - 1, static_cast<int>(std::ceil(std::max({p0.x, p1.x, p2.x}))));
    const int minY = std::max(0, static_cast<int>(std::floor(std::min({p0.y, p1.y, p2.y}))));
    const int maxY = std::min(height - 1, static_cast<int>(std::ceil(std::max({p0.y, p1.y, p2.y}))));

    const int startX = std::max(minX, tileMinX);
    const int endX = std::min(maxX, tileMaxX);
    const int startY = std::max(minY, tileMinY);
    const int endY = std::min(maxY, tileMaxY);

    if (startX > endX || startY > endY)
        return 0;

    int pixelsWritten = 0;

    auto rasterizePixel = [&](int x, int y)
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

            const float interInvW = w0 * v0.invW + w1 * v1.invW + w2 * v2.invW;
            const float cW0 = (w0 * v0.invW) / interInvW;
            const float cW1 = (w1 * v1.invW) / interInvW;
            const float cW2 = (w2 * v2.invW) / interInvW;
            const float depth = cW0 * v0.depth + cW1 * v1.depth + cW2 * v2.depth;

            const Vector2 uv = (v0.uv * (w0 * v0.invW) + v1.uv * (w1 * v1.invW) + v2.uv * (w2 * v2.invW)) / interInvW;
            const Vector3 normal = (v0.normal * (w0 * v0.invW) + v1.normal * (w1 * v1.invW) + v2.normal * (w2 * v2.invW)) / interInvW;
            const Vector3 worldPos = (v0.worldPosition * (w0 * v0.invW) + v1.worldPosition * (w1 * v1.invW) + v2.worldPosition * (w2 * v2.invW)) / interInvW;
            const Vector3 viewDir = (camera.position - worldPos).Normalized();
            const Vector3 lightDir = (-light.direction).Normalized();

            const Material *material = nullptr;
            if (materialIndex >= 0 && materialIndex < static_cast<int>(mesh.materials.size()))
                material = &mesh.materials[materialIndex];

            Color baseColor = {255,255,255};
            if (material && material->hasDiffuseMap)
                baseColor = material->diffuseMap.Sample(uv);
            else if (mesh.hasTexture)
                baseColor = mesh.texture.Sample(uv);
            else if (material)
                baseColor = material->diffuse;

            float ambient = material ? material->ambient.r / 255.0f : 0.1f;
            float specularStrength = material ? material->specular.r / 255.0f : 0.5f;
            float shininess = material ? material->shininess : 32.0f;

            const float diffuse = std::max(0.0f, Vector3::Dot(normal.Normalized(), lightDir));
            const Vector3 reflectDir = (normal.Normalized() * (2.0f * Vector3::Dot(normal.Normalized(), lightDir)) - lightDir).Normalized();
            const float specular = std::pow(std::max(0.0f, Vector3::Dot(viewDir, reflectDir)), shininess);

            float brightness = ambient + diffuse + specularStrength * specular;
            brightness = std::clamp(brightness, 0.0f, 1.0f);

            Color color = {
                static_cast<unsigned char>(baseColor.r * brightness),
                static_cast<unsigned char>(baseColor.g * brightness),
                static_cast<unsigned char>(baseColor.b * brightness)
            };

            if (useLock)
            {
                WriteFragment(x, y, depth, color);
            }
            else
            {
                WriteFragmentUnlocked(x, y, depth, color);
                ++pixelsWritten;
            }
        }
    };

    if (useTiles)
    {
        const int minTileX = std::max(0, startX / TileSize);
        const int maxTileX = std::min((width + TileSize - 1) / TileSize - 1, endX / TileSize);
        const int minTileY = std::max(0, startY / TileSize);
        const int maxTileY = std::min((height + TileSize - 1) / TileSize - 1, endY / TileSize);

        for (int tileY = minTileY; tileY <= maxTileY; ++tileY)
        {
            for (int tileX = minTileX; tileX <= maxTileX; ++tileX)
            {
                const int tileMinXInner = tileX * TileSize;
                const int tileMaxXInner = std::min(width - 1, tileMinXInner + TileSize - 1);
                const int tileMinYInner = tileY * TileSize;
                const int tileMaxYInner = std::min(height - 1, tileMinYInner + TileSize - 1);

                const int scanX0 = std::max(startX, tileMinXInner);
                const int scanX1 = std::min(endX, tileMaxXInner);
                const int scanY0 = std::max(startY, tileMinYInner);
                const int scanY1 = std::min(endY, tileMaxYInner);

                for (int y = scanY0; y <= scanY1; ++y)
                {
                    if (useSIMD)
                    {
#if defined(__AVX__)
                        int x = scanX0;
                        while (x + 7 <= scanX1)
                        {
                            __m256 px = _mm256_set_ps(static_cast<float>(x + 7), static_cast<float>(x + 6), static_cast<float>(x + 5), static_cast<float>(x + 4), static_cast<float>(x + 3), static_cast<float>(x + 2), static_cast<float>(x + 1), static_cast<float>(x));
                            __m256 py = _mm256_set1_ps(static_cast<float>(y) + 0.5f);
                            __m256 p1x = _mm256_set1_ps(p1.x);
                            __m256 p1y = _mm256_set1_ps(p1.y);
                            __m256 p2x = _mm256_set1_ps(p2.x);
                            __m256 p2y = _mm256_set1_ps(p2.y);
                            __m256 p0x = _mm256_set1_ps(p0.x);
                            __m256 p0y = _mm256_set1_ps(p0.y);

                            __m256 w0v = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(py, p1y), _mm256_sub_ps(p2x, p1x)), _mm256_mul_ps(_mm256_sub_ps(px, p1x), _mm256_sub_ps(p2y, p1y)));
                            __m256 w1v = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(py, p2y), _mm256_sub_ps(p0x, p2x)), _mm256_mul_ps(_mm256_sub_ps(px, p2x), _mm256_sub_ps(p0y, p2y)));
                            __m256 w2v = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(py, p0y), _mm256_sub_ps(p1x, p0x)), _mm256_mul_ps(_mm256_sub_ps(px, p0x), _mm256_sub_ps(p1y, p0y)));

                            float laneValues[8];
                            _mm256_storeu_ps(laneValues, w0v);
                            float laneValues1[8];
                            _mm256_storeu_ps(laneValues1, w1v);
                            float laneValues2[8];
                            _mm256_storeu_ps(laneValues2, w2v);

                            for (int lane = 0; lane < 8; ++lane)
                            {
                                const float w0 = laneValues[lane];
                                const float w1 = laneValues1[lane];
                                const float w2 = laneValues2[lane];
                                const int pixelX = x + lane;
                                const bool inside = (area >= 0.0f && w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
                                                    (area < 0.0f && w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f);

                                if (!inside)
                                    continue;

                                rasterizePixel(pixelX, y);
                            }

                            x += 8;
                        }
#elif defined(__SSE__)
                        int x = scanX0;
                        while (x + 3 <= scanX1)
                        {
                            float x0 = static_cast<float>(x);
                            float x1 = static_cast<float>(x + 1);
                            float x2 = static_cast<float>(x + 2);
                            float x3 = static_cast<float>(x + 3);
                            float yv = static_cast<float>(y) + 0.5f;

                            __m128 px = _mm_set_ps(x3, x2, x1, x0);
                            __m128 py = _mm_set1_ps(yv);
                            __m128 p1x = _mm_set1_ps(p1.x);
                            __m128 p1y = _mm_set1_ps(p1.y);
                            __m128 p2x = _mm_set1_ps(p2.x);
                            __m128 p2y = _mm_set1_ps(p2.y);
                            __m128 p0x = _mm_set1_ps(p0.x);
                            __m128 p0y = _mm_set1_ps(p0.y);

                            __m128 w0v = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(py, p1y), _mm_sub_ps(p2x, p1x)), _mm_mul_ps(_mm_sub_ps(px, p1x), _mm_sub_ps(p2y, p1y)));
                            __m128 w1v = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(py, p2y), _mm_sub_ps(p0x, p2x)), _mm_mul_ps(_mm_sub_ps(px, p2x), _mm_sub_ps(p0y, p2y)));
                            __m128 w2v = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(py, p0y), _mm_sub_ps(p1x, p0x)), _mm_mul_ps(_mm_sub_ps(px, p0x), _mm_sub_ps(p1y, p0y)));

                            float laneValues[4];
                            _mm_storeu_ps(laneValues, w0v);
                            float laneValues1[4];
                            _mm_storeu_ps(laneValues1, w1v);
                            float laneValues2[4];
                            _mm_storeu_ps(laneValues2, w2v);

                            for (int lane = 0; lane < 4; ++lane)
                            {
                                const float w0 = laneValues[lane];
                                const float w1 = laneValues1[lane];
                                const float w2 = laneValues2[lane];
                                const int pixelX = x + lane;
                                const bool inside = (area >= 0.0f && w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
                                                    (area < 0.0f && w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f);

                                if (!inside)
                                    continue;

                                rasterizePixel(pixelX, y);
                            }

                            x += 4;
                        }
#endif
                    }

                    for (int x = scanX0; x <= scanX1; ++x)
                    {
                        rasterizePixel(x, y);
                    }
                }
            }
        }
    }
    else
    {
        for (int y = startY; y <= endY; ++y)
        {
            for (int x = startX; x <= endX; ++x)
            {
                rasterizePixel(x, y);
            }
        }
    }

    return pixelsWritten;
}

void Renderer::RenderMesh(const Mesh &mesh)
{
    stats.meshes++;
    GeometryStage(mesh);
}

void Renderer::GeometryStage(const Mesh &mesh)
{
    visibleTriangles.reserve(visibleTriangles.size() + mesh.indices.size() / 3);

    for (size_t i = 0; i < mesh.indices.size(); i += 3)
    {
        stats.triangles++;
        stats.vertices += 3;

        Triangle triangle;

        triangle.v0 = TransformVertex(mesh.vertices[mesh.indices[i]], mesh.transform);
        triangle.v1 = TransformVertex(mesh.vertices[mesh.indices[i + 1]], mesh.transform);
        triangle.v2 = TransformVertex(mesh.vertices[mesh.indices[i + 2]], mesh.transform);

        std::vector<Triangle> input;
        std::vector<Triangle> output;

        input.push_back(triangle);

        auto ClipStage = [&](const Plane &plane)
        {
            output.clear();

            for (const Triangle &t : input)
                PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(t, plane, output);

            input = output;

            if (input.empty())
                return;
        };

        Plane nearPlane = NormalizePlane({{0.0f, 0.0f, 1.0f},
                                          -0.1f});
        Plane farPlane = NormalizePlane({{0.0f, 0.0f, -1.0f},
                                         10000.0f});
        Plane leftPlane = NormalizePlane({{1.0f, 0.0f, 1.0f},
                                          0.0f});

        Plane rightPlane = NormalizePlane({{-1.0f, 0.0f, 1.0f},
                                           0.0f});

        Plane topPlane = NormalizePlane({{0.0f, -1.0f, 1.0f},
                                         0.0f});

        Plane bottomPlane = NormalizePlane({{0.0f, 1.0f, 1.0f},
                                            0.0f});
        ClipStage(nearPlane);
        if (input.empty())
            continue;
        ClipStage(farPlane);
        if (input.empty())
            continue;
        ClipStage(leftPlane);
        if (input.empty())
            continue;
        ClipStage(rightPlane);
        if (input.empty())
            continue;
        ClipStage(topPlane);
        if (input.empty())
            continue;
        ClipStage(bottomPlane);
        if (input.empty())
            continue;

        for (const Triangle &clippedTriangle : input)
        {
            Vertex v0 = clippedTriangle.v0;
            Vertex v1 = clippedTriangle.v1;
            Vertex v2 = clippedTriangle.v2;

            Vector2 p0 = ProjectVertex(v0);
            Vector2 p1 = ProjectVertex(v1);
            Vector2 p2 = ProjectVertex(v2);

            if (IsBackFace(p0, p1, p2))
            {
                stats.trianglesCulled++;
                continue;
            }

            int triangleMaterialIndex = static_cast<int>(i / 3);
            int materialIndex = -1;
            if (triangleMaterialIndex < static_cast<int>(mesh.materialIndices.size()))
                materialIndex = mesh.materialIndices[triangleMaterialIndex];

            visibleTriangles.push_back({clippedTriangle, &mesh, materialIndex});
        }
    }
}

void Renderer::RasterizeVisibleTriangles()
{
    if (renderMode != RenderMode::Filled)
    {
        for (VisibleTriangle &visibleTriangle : visibleTriangles)
        {
            Vertex v0 = visibleTriangle.triangle.v0;
            Vertex v1 = visibleTriangle.triangle.v1;
            Vertex v2 = visibleTriangle.triangle.v2;

            Vector2 p0 = ProjectVertex(v0);
            Vector2 p1 = ProjectVertex(v1);
            Vector2 p2 = ProjectVertex(v2);

            DrawLine(p0, p1, v0.color);
            DrawLine(p1, p2, v1.color);
            DrawLine(p2, p0, v2.color);
        }

        visibleTriangles.clear();
        return;
    }

    if (visibleTriangles.empty())
    {
        return;
    }

    const unsigned int threadCount = std::max(1u, std::min(8u, std::thread::hardware_concurrency()));
    const size_t triangleCount = visibleTriangles.size();
    stats.activeThreads = 1;

    if (!useMultithreading || threadCount <= 1 || triangleCount < 2 || !useTiles)
    {
        for (VisibleTriangle &visibleTriangle : visibleTriangles)
        {
            RasterizeTriangleViewSpace(visibleTriangle.triangle, *visibleTriangle.mesh);
        }

        visibleTriangles.clear();
        return;
    }

    const int tilesX = (width + TileSize - 1) / TileSize;
    const int tilesY = (height + TileSize - 1) / TileSize;
    const int totalTiles = tilesX * tilesY;
    const int workerCount = std::min<int>(static_cast<int>(threadCount), totalTiles);
    const int tilesPerThread = std::max(1, totalTiles / workerCount);

    std::vector<std::vector<size_t>> tileTriangles(totalTiles);
    for (size_t triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
    {
        const VisibleTriangle &visibleTriangle = visibleTriangles[triangleIndex];
        const Triangle &triangle = visibleTriangle.triangle;

        Vertex v0 = triangle.v0;
        Vertex v1 = triangle.v1;
        Vertex v2 = triangle.v2;

        Vector2 p0 = ProjectVertex(v0);
        Vector2 p1 = ProjectVertex(v1);
        Vector2 p2 = ProjectVertex(v2);

        const int minX = std::max(0, static_cast<int>(std::floor(std::min({p0.x, p1.x, p2.x}))));
        const int maxX = std::min(width - 1, static_cast<int>(std::ceil(std::max({p0.x, p1.x, p2.x}))));
        const int minY = std::max(0, static_cast<int>(std::floor(std::min({p0.y, p1.y, p2.y}))));
        const int maxY = std::min(height - 1, static_cast<int>(std::ceil(std::max({p0.y, p1.y, p2.y}))));

        const int minTileX = std::max(0, minX / TileSize);
        const int maxTileX = std::min(tilesX - 1, maxX / TileSize);
        const int minTileY = std::max(0, minY / TileSize);
        const int maxTileY = std::min(tilesY - 1, maxY / TileSize);

        for (int tileY = minTileY; tileY <= maxTileY; ++tileY)
        {
            for (int tileX = minTileX; tileX <= maxTileX; ++tileX)
            {
                tileTriangles[tileY * tilesX + tileX].push_back(triangleIndex);
            }
        }
    }

    std::vector<std::thread> workers;
    workers.reserve(workerCount);
    stats.activeThreads = workerCount;

    int tileBegin = 0;
    while (tileBegin < totalTiles)
    {
        const int tileEnd = std::min(tileBegin + tilesPerThread, totalTiles);
        workers.emplace_back([this, tileBegin, tileEnd, tilesX, tilesY, &tileTriangles]()
                             {
                                 for (int tileIndex = tileBegin; tileIndex < tileEnd; ++tileIndex)
                                 {
                                     const int tileX = tileIndex % tilesX;
                                     const int tileY = tileIndex / tilesX;
                                     const int tileMinX = tileX * TileSize;
                                     const int tileMaxX = std::min(width - 1, tileMinX + TileSize - 1);
                                     const int tileMinY = tileY * TileSize;
                                     const int tileMaxY = std::min(height - 1, tileMinY + TileSize - 1);
                                     const auto &trianglesInTile = tileTriangles[tileIndex];

                                     for (size_t triangleIndex : trianglesInTile)
                                     {
                                         const VisibleTriangle &visibleTriangle = visibleTriangles[triangleIndex];
                                         RasterizeTriangleViewSpace(visibleTriangle.triangle, *visibleTriangle.mesh, tileMinX, tileMaxX, tileMinY, tileMaxY, false);
                                     }
                                 }
                             });
        tileBegin = tileEnd;
    }

    for (std::thread &worker : workers)
    {
        worker.join();
    }

    visibleTriangles.clear();
}

void Renderer::WriteFragment(int x, int y, float depth, const Color &color)
{
    std::lock_guard<std::mutex> lock(pixelMutex);

    const int index = y * width + x;
    if (depth < depthBuffer[index])
    {
        depthBuffer[index] = depth;
        framebufer[index] = color;
        stats.pixelsDrawn++;
    }
}

void Renderer::WriteFragmentUnlocked(int x, int y, float depth, const Color &color)
{
    const int index = y * width + x;
    if (depth < depthBuffer[index])
    {
        depthBuffer[index] = depth;
        framebufer[index] = color;
        stats.pixelsDrawn++;
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

    Vector3 worldNormal = model.MultiplyVector(vertex.normal).Normalized();

    result.position = view;
    result.normal = worldNormal;
    result.worldPosition = world;

    return result;
}

void Renderer::PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(const Triangle &triangle, const Plane &plane, std::vector<Triangle> &output)
{
    Vertex verts[3] =
        {
            triangle.v0,
            triangle.v1,
            triangle.v2};

    std::vector<Vertex> clippedPolygon;

    for (int i = 0; i < 3; i++)
    {
        const Vertex &current = verts[i];
        const Vertex &next = verts[(i + 1) % 3];

        float distCurrent = DistanceToPlane(plane, current.position);
        float distNext = DistanceToPlane(plane, next.position);

        if (distCurrent >= 0.0f)
        {
            clippedPolygon.push_back(current);

            if (distNext < 0.0f)
            {
                clippedPolygon.push_back(IntersectPlane(current, next, plane));
            }
        }

        else if (distNext >= 0.0f)
        {
            clippedPolygon.push_back(IntersectPlane(current, next, plane));
        }
    }

    if (clippedPolygon.size() < 3)
        return;

    for (size_t i = 1; i < clippedPolygon.size() - 1; i++)
    {
        Triangle t;
        t.v0 = clippedPolygon[0];
        t.v1 = clippedPolygon[i];
        t.v2 = clippedPolygon[i + 1];
        output.push_back(t);
    }
}