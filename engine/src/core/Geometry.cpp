#include "core/Geometry.hpp"
#include <glm/gtc/constants.hpp>
#include <tuple>
#include <vector>

std::wstring Geometry::name[Geometry::Type::END + 1] = {L"球体",   L"立方体", L"圆柱体", L"圆锥体", L"棱柱",
                                                        L"金字塔", L"环面",   L"椭球体", L"截头锥"};

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateSphereData(float radius, int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = glm::pi<float>();
    const float TWO_PI = 2.0f * PI;

    for (int i = 0; i <= segments; ++i)
    {
        float v = i / (float)segments;
        float phi = v * PI;

        for (int j = 0; j <= segments; ++j)
        {
            float u = j / (float)segments;
            float theta = u * TWO_PI;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            glm::vec3 position(x, y, z);
            glm::vec3 normal = glm::normalize(position);
            glm::vec2 texCoord(u, v);

            vertices.push_back({position, normal, texCoord});
        }
    }

    for (int i = 0; i < segments; ++i)
    {
        for (int j = 0; j < segments; ++j)
        {
            int first = i * (segments + 1) + j;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);

            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second + 1);
        }
    }
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateSphere(float radius, int segments)
{
    auto [vertices, indices] = GenerateSphereData(radius, segments);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateCubeData(float width, float height,
                                                                                      float depth)
{
    std::vector<Vertex> vertices = {
        // 前面 (Z正方向)
        {{-width, -height, depth}, {0, 0, 1}, {0, 0}}, // 0 左下
        {{width, -height, depth}, {0, 0, 1}, {1, 0}},  // 1 右下
        {{width, height, depth}, {0, 0, 1}, {1, 1}},   // 2 右上
        {{-width, height, depth}, {0, 0, 1}, {0, 1}},  // 3 左上

        // 后面 (Z负方向)
        {{width, -height, -depth}, {0, 0, -1}, {0, 0}},  // 4 右下
        {{-width, -height, -depth}, {0, 0, -1}, {1, 0}}, // 5 左下
        {{-width, height, -depth}, {0, 0, -1}, {1, 1}},  // 6 左上
        {{width, height, -depth}, {0, 0, -1}, {0, 1}},   // 7 右上

        // 左面 (X负方向)
        {{-width, -height, -depth}, {-1, 0, 0}, {0, 0}}, // 8 后下
        {{-width, -height, depth}, {-1, 0, 0}, {1, 0}},  // 9 前下
        {{-width, height, depth}, {-1, 0, 0}, {1, 1}},   // 10 前上
        {{-width, height, -depth}, {-1, 0, 0}, {0, 1}},  // 11 后上

        // 右面 (X正方向)
        {{width, -height, depth}, {1, 0, 0}, {0, 0}},  // 12 前下
        {{width, -height, -depth}, {1, 0, 0}, {1, 0}}, // 13 后下
        {{width, height, -depth}, {1, 0, 0}, {1, 1}},  // 14 后上
        {{width, height, depth}, {1, 0, 0}, {0, 1}},   // 15 前上

        // 顶面 (Y正方向)
        {{-width, height, depth}, {0, 1, 0}, {0, 1}},  // 16 前左
        {{width, height, depth}, {0, 1, 0}, {1, 1}},   // 17 前右
        {{width, height, -depth}, {0, 1, 0}, {1, 0}},  // 18 后右
        {{-width, height, -depth}, {0, 1, 0}, {0, 0}}, // 19 后左

        // 底面 (Y负方向)
        {{-width, -height, -depth}, {0, -1, 0}, {0, 0}}, // 20 后左
        {{width, -height, -depth}, {0, -1, 0}, {1, 0}},  // 21 后右
        {{width, -height, depth}, {0, -1, 0}, {1, 1}},   // 22 前右
        {{-width, -height, depth}, {0, -1, 0}, {0, 1}}   // 23 前左
    };

    std::vector<unsigned int> indices = {// 前面 (逆时针: 左下->右下->右上->左上)
                                         0, 1, 2, 0, 2, 3,

                                         // 后面 (逆时针: 右下->左下->左上->右上)
                                         4, 5, 6, 4, 6, 7,

                                         // 左面 (逆时针: 后下->前下->前上->后上)
                                         8, 9, 10, 8, 10, 11,

                                         // 右面 (逆时针: 前下->后下->后上->前上)
                                         12, 13, 14, 12, 14, 15,

                                         // 顶面 (逆时针: 前左->前右->后右->后左)
                                         16, 17, 18, 16, 18, 19,

                                         // 底面 (逆时针: 后左->后右->前右->前左)
                                         20, 21, 22, 20, 22, 23};
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCube(float width, float height, float depth)
{
    auto [vertices, indices] = GenerateCubeData(width, height, depth);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateCylinderData(float radius, float height,
                                                                                          int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;

    // 生成顶点
    for (int i = 0; i <= segments; ++i)
    {
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);

        // 侧面法线（从圆柱中心向外辐射）
        glm::vec3 sideNormal = glm::normalize(glm::vec3(x, 0, z));

        // 顶部顶点（法线朝上）
        vertices.push_back({{x, halfHeight, z}, {0, 1, 0}, {i / (float)segments, 0}});

        // 底部顶点（法线朝下）
        vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {i / (float)segments, 1}});

        // 侧面顶部顶点（使用侧面法线）
        vertices.push_back({{x, halfHeight, z}, sideNormal, {i / (float)segments, 0.5f}});

        // 侧面底部顶点（使用侧面法线）
        vertices.push_back({{x, -halfHeight, z}, sideNormal, {i / (float)segments, 0.5f}});
    }

    // 生成索引（绘制侧面）
    for (int i = 0; i < segments; ++i)
    {
        int topSide1 = i * 4 + 2;          // 侧面顶部顶点1
        int topSide2 = (i + 1) * 4 + 2;    // 侧面顶部顶点2
        int bottomSide1 = i * 4 + 3;       // 侧面底部顶点1
        int bottomSide2 = (i + 1) * 4 + 3; // 侧面底部顶点2

        // 侧面四边形（由两个三角形组成）
        indices.push_back(topSide1);
        indices.push_back(topSide2);
        indices.push_back(bottomSide1);

        indices.push_back(topSide2);
        indices.push_back(bottomSide2);
        indices.push_back(bottomSide1);
    }
    // 生成索引（绘制顶部和底部）
    // 生成顶部圆面索引
    for (int i = 0; i < segments; ++i)
    {
        indices.push_back(0); // 顶部中心顶点（需要单独添加）
        indices.push_back((i + 1) * 4);
        indices.push_back(i * 4);
    }

    // 生成底部圆面索引
    for (int i = 0; i < segments; ++i)
    {
        indices.push_back(1); // 底部中心顶点（需要单独添加）
        indices.push_back(i * 4 + 1);
        indices.push_back((i + 1) * 4 + 1);
    }
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCylinder(float radius, float height, int segments)
{
    auto [vertices, indices] = GenerateCylinderData(radius, height, segments);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateConeData(float radius, float height,
                                                                                      int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;

    // 圆锥底部中心顶点（用于底部圆面）
    vertices.push_back({{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}});

    // 生成底部圆面和侧面顶点
    for (int i = 0; i <= segments; ++i)
    {
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);

        // 底部圆面顶点（法线朝下）
        vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta)}});

        // 侧面顶点（法线需要计算）
        glm::vec3 sideNormal = glm::normalize(glm::vec3(x, radius / height, z)); // 圆锥侧面法线
        vertices.push_back({{x, -halfHeight, z}, sideNormal, {i / (float)segments, 1}});
    }

    // 圆锥顶部顶点（法线朝上）
    vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0}});

    // 生成侧面索引（确保逆时针顺序）
    for (int i = 0; i < segments; ++i)
    {
        int bottom1 = 1 + i * 2;                    // 底部顶点（圆面）
        int bottom2 = 1 + ((i + 1) % segments) * 2; // 下一个底部顶点
        int side1 = 2 + i * 2;                      // 侧面顶点1
        int side2 = 2 + ((i + 1) % segments) * 2;   // 侧面顶点2
        int top = vertices.size() - 1;              // 顶部顶点

        // 侧面三角形（逆时针顺序）
        indices.push_back(side1);
        indices.push_back(top);
        indices.push_back(side2);

        // 底部圆面三角形（逆时针顺序）
        indices.push_back(0); // 底部中心
        indices.push_back(bottom1);
        indices.push_back(bottom2);
    }
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCone(float radius, float height, int segments)
{
    auto [vertices, indices] = GenerateConeData(radius, height, segments);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GeneratePrismData(int sides, float radius,
                                                                                       float height)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;
    float angleStep = 2.0f * glm::pi<float>() / sides;

    for (int i = 0; i <= sides; ++i) // 注意：i <= sides 以确保闭合
    {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // 侧面法线（垂直于棱柱侧面）
        glm::vec3 sideNormal = glm::normalize(glm::vec3(x, 0, z));

        // 顶部顶点（法线朝上）
        vertices.push_back({{x, halfHeight, z}, {0, 1, 0}, {i / (float)sides, 0}});
        // 底部顶点（法线朝下）
        vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {i / (float)sides, 1}});
        // 侧面顶部顶点（使用侧面法线）
        vertices.push_back({{x, halfHeight, z}, sideNormal, {i / (float)sides, 0.5f}});
        // 侧面底部顶点（使用侧面法线）
        vertices.push_back({{x, -halfHeight, z}, sideNormal, {i / (float)sides, 0.5f}});
    }

    // 生成索引
    for (int i = 0; i < sides; ++i)
    {
        // 顶部圆面（逆时针）
        int top1 = i * 4;
        int top2 = (i + 1) * 4;
        indices.push_back(top2);
        indices.push_back(top1);
        indices.push_back(0);

        // 底部圆面（顺时针，因为法线朝下）
        int bottom1 = i * 4 + 1;
        int bottom2 = (i + 1) * 4 + 1;
        indices.push_back(1);
        indices.push_back(bottom1);
        indices.push_back(bottom2);

        // 侧面四边形（由两个三角形组成，逆时针）
        int sideTop1 = i * 4 + 2;
        int sideTop2 = (i + 1) * 4 + 2;
        int sideBottom1 = i * 4 + 3;
        int sideBottom2 = (i + 1) * 4 + 3;

        // 第一个三角形
        indices.push_back(sideTop1);
        indices.push_back(sideTop2);
        indices.push_back(sideBottom1);

        // 第二个三角形
        indices.push_back(sideTop2);
        indices.push_back(sideBottom2);
        indices.push_back(sideBottom1);
    }
    return std::make_tuple(vertices, indices);
}
std::shared_ptr<Mesh> Geometry::CreatePrism(int sides, float radius, float height)
{
    auto [vertices, indices] = GeneratePrismData(sides, radius, height);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GeneratePyramidData(int sides, float baseSize,
                                                                                         float height)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfBase = baseSize / 2.0f;
    float angleStep = 2.0f * glm::pi<float>() / sides;

    // 添加底部中心顶点（用于底部面）
    vertices.push_back({{0, 0, 0}, {0, -1, 0}, {0.5f, 0.5f}});

    // 生成底部和侧面顶点
    for (int i = 0; i <= sides; ++i) // 使用<=确保闭合
    {
        float angle = i * angleStep;
        float x = halfBase * cos(angle);
        float z = halfBase * sin(angle);

        // 底部边缘顶点（法线朝下）
        vertices.push_back({{x, 0, z}, {0, -1, 0}, {0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)}});

        // 侧面顶点（需要计算法线）
        glm::vec3 sideNormal = glm::normalize(glm::vec3(x, height, z));
        vertices.push_back({{x, 0, z}, sideNormal, {i / (float)sides, 1.0f}});
    }

    // 顶部顶点
    int topIndex = vertices.size();
    vertices.push_back({{0, height, 0}, glm::vec3(0, 1, 0), {0.5f, 0.0f}});

    // 生成索引
    for (int i = 0; i < sides; ++i)
    {
        // 底部三角形（逆时针）
        int center = 0;
        int bottom1 = 1 + i * 2;
        int bottom2 = 1 + ((i + 1) % sides) * 2;
        indices.push_back(center);
        indices.push_back(bottom1);
        indices.push_back(bottom2);

        // 侧面三角形（逆时针）
        int side1 = 2 + i * 2;
        int side2 = 2 + ((i + 1) % sides) * 2;
        indices.push_back(side1);
        indices.push_back(topIndex);
        indices.push_back(side2);
    }
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreatePyramid(int sides, float baseSize, float height)
{
    auto [vertices, indices] = GeneratePyramidData(sides, baseSize, height);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateTorusData(float majorRadius,
                                                                                       float minorRadius,
                                                                                       int majorSegments,
                                                                                       int minorSegments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float majorStep = 2.0f * glm::pi<float>() / majorSegments;
    const float minorStep = 2.0f * glm::pi<float>() / minorSegments;

    // 生成顶点
    for (int i = 0; i <= majorSegments; ++i)
    {
        float theta = i * majorStep;
        glm::vec2 majorCircle(cos(theta), sin(theta));

        for (int j = 0; j <= minorSegments; ++j)
        {
            float phi = j * minorStep;
            glm::vec2 minorCircle(cos(phi), sin(phi));

            // 计算顶点位置
            glm::vec3 position((majorRadius + minorRadius * minorCircle.x) * majorCircle.x, minorRadius * minorCircle.y,
                               (majorRadius + minorRadius * minorCircle.x) * majorCircle.y);

            // 计算法线（从主环中心指向圆环表面）
            glm::vec3 normal(minorCircle.x * majorCircle.x, minorCircle.y, minorCircle.x * majorCircle.y);

            // 优化纹理坐标
            glm::vec2 texCoord(static_cast<float>(i) / majorSegments, static_cast<float>(j) / minorSegments);

            vertices.push_back({position, normal, texCoord});
        }
    }

    // 生成索引（确保逆时针顺序）
    for (int i = 0; i < majorSegments; ++i)
    {
        for (int j = 0; j < minorSegments; ++j)
        {
            int current = i * (minorSegments + 1) + j;
            int next = current + minorSegments + 1;

            // 第一个三角形（逆时针）
            indices.push_back(current);
            indices.push_back(current + 1);
            indices.push_back(next);

            // 第二个三角形（逆时针）
            indices.push_back(next);
            indices.push_back(current + 1);
            indices.push_back(next + 1);
        }
    }

    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments)
{
    auto [vertices, indices] = GenerateTorusData(majorRadius, minorRadius, majorSegments, minorSegments);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateEllipsoidData(float radiusX, float radiusY,
                                                                                           float radiusZ, int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = glm::pi<float>();
    const float segmentStep = PI / segments;
    const float ringStep = 2.0f * PI / segments;

    // 预分配内存提高性能
    vertices.reserve((segments + 1) * (segments + 1));
    indices.reserve(segments * segments * 6);

    // 生成顶点
    for (int ring = 0; ring <= segments; ++ring) // 纬度环
    {
        float phi = ring * segmentStep; // [0, π]
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        for (int segment = 0; segment <= segments; ++segment) // 经度段
        {
            float theta = segment * ringStep; // [0, 2π]
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            // 计算顶点位置
            glm::vec3 position(radiusX * sinPhi * cosTheta, radiusY * cosPhi, radiusZ * sinPhi * sinTheta);

            // 精确计算法线（考虑不同半径）
            glm::vec3 normal(position.x / (radiusX * radiusX), position.y / (radiusY * radiusY),
                             position.z / (radiusZ * radiusZ));
            normal = glm::normalize(normal);

            // 优化纹理坐标
            glm::vec2 texCoord(static_cast<float>(segment) / segments, static_cast<float>(ring) / segments);

            vertices.push_back({position, normal, texCoord});
        }
    }

    // 生成索引（确保逆时针顺序）
    for (int ring = 0; ring < segments; ++ring)
    {
        for (int segment = 0; segment < segments; ++segment)
        {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;

            // 第一个三角形（逆时针）
            indices.push_back(current);
            indices.push_back(current + 1);
            indices.push_back(next);

            // 第二个三角形（逆时针）
            indices.push_back(next);
            indices.push_back(current + 1);
            indices.push_back(next + 1);
        }
    }
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateEllipsoid(float radiusX, float radiusY, float radiusZ, int segments)
{
    auto [vertices, indices] = GenerateEllipsoidData(radiusX, radiusY, radiusZ, segments);
    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateFrustumData(float radiusTop,
                                                                                         float radiusBottom,
                                                                                         float height, int segments)
{
    if (segments < 3)
        segments = 3; // 确保至少3个分段

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float halfHeight = height / 2.0f;
    const float slope = (radiusBottom - radiusTop) / height;

    // 1. 创建顶部和底部中心顶点
    vertices.push_back({{0.0f, halfHeight, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}});   // 顶部中心 (0)
    vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.0f}}); // 底部中心 (1)

    // 2. 创建顶部和底部圆环顶点
    for (int i = 0; i < segments; ++i)
    {
        const float theta = i * 2.0f * glm::pi<float>() / segments;
        const float cosTheta = cos(theta);
        const float sinTheta = sin(theta);

        // 顶部圆环顶点 (法线向上)
        vertices.push_back({
            {radiusTop * cosTheta, halfHeight, radiusTop * sinTheta},
            {0.0f, 1.0f, 0.0f},
            {0.5f + 0.5f * cosTheta, 0.5f + 0.5f * sinTheta} // 圆形展开
        });

        // 底部圆环顶点 (法线向下)
        vertices.push_back({
            {radiusBottom * cosTheta, -halfHeight, radiusBottom * sinTheta},
            {0.0f, -1.0f, 0.0f},
            {0.5f + 0.5f * cosTheta, 0.5f + 0.5f * sinTheta} // 圆形展开
        });
    }

    // 3. 创建侧面顶点 (每个顶点有独立的法线)
    for (int i = 0; i < segments; ++i)
    {
        const float theta = i * 2.0f * glm::pi<float>() / segments;
        const float cosTheta = cos(theta);
        const float sinTheta = sin(theta);

        // 计算侧面法线 (基于斜率)
        glm::vec3 normal(cosTheta, slope, sinTheta);
        normal = glm::normalize(normal);

        // 顶部环侧面顶点
        vertices.push_back({
            {radiusTop * cosTheta, halfHeight, radiusTop * sinTheta}, normal, {i / static_cast<float>(segments), 1.0f}
            // U沿圆周，V=1.0顶部
        });

        // 底部环侧面顶点
        vertices.push_back({
            {radiusBottom * cosTheta, -halfHeight, radiusBottom * sinTheta},
            normal,
            {i / static_cast<float>(segments), 0.0f} // U沿圆周，V=0.0底部
        });
    }

    // 4. 创建索引 (确保逆时针顺序)

    // 顶部圆面 (逆时针)
    for (int i = 0; i < segments; ++i)
    {
        const int next = (i + 1) % segments;
        indices.push_back(0);            // 顶部中心
        indices.push_back(2 + 2 * next); // 下一个顶部点
        indices.push_back(2 + 2 * i);    // 当前顶部点
    }

    // 底部圆面 (逆时针)
    for (int i = 0; i < segments; ++i)
    {
        const int next = (i + 1) % segments;
        indices.push_back(1);                // 底部中心
        indices.push_back(2 + 2 * i + 1);    // 当前底部点
        indices.push_back(2 + 2 * next + 1); // 下一个底部点
    }

    // 侧面 (逆时针)
    const int sideStart = 2 + 2 * segments; // 侧面顶点起始索引
    for (int i = 0; i < segments; ++i)
    {
        const int next = (i + 1) % segments;

        // 当前四边形
        const int topCurrent = sideStart + 2 * i;
        const int bottomCurrent = topCurrent + 1;
        const int topNext = sideStart + 2 * next;
        const int bottomNext = topNext + 1;

        // 第一个三角形 (顶部当前 -> 底部当前 -> 顶部下一个)
        indices.push_back(topCurrent);
        indices.push_back(topNext);
        indices.push_back(bottomCurrent);

        // 第二个三角形 (底部当前 -> 底部下一个 -> 顶部下一个)
        indices.push_back(bottomCurrent);
        indices.push_back(topNext);
        indices.push_back(bottomNext);
    }
    return std::make_tuple(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateFrustum(float radiusTop, float radiusBottom, float height, int segments)
{
    auto [vertices, indices] = GenerateFrustumData(radiusTop, radiusBottom, height, segments);

    return std::make_shared<Mesh>(vertices, indices);
}

std::tuple<std::vector<Vertex>, std::vector<unsigned int>> Geometry::GenerateArrowData(float length, float headSize)
{
    // 创建箭头的主体部分（圆柱体）
    auto [cylinderVertices, cylinderIndices] = GenerateCylinderData(0.05f, length - headSize, 16);

    // 创建箭头的头部部分（圆锥体）
    auto [coneVertices, coneIndices] = GenerateConeData(0.1f, headSize, 16);

    // 设置圆锥的位置，使其位于圆柱的顶部
    glm::vec3 conePosition(0.0f, (length - headSize) / 2.0f + headSize / 2.0f, 0.0f);

    // 合并圆柱和圆锥的顶点和索引
    std::vector<Vertex> combinedVertices = cylinderVertices;
    std::vector<unsigned int> combinedIndices = cylinderIndices;

    // 更新圆锥的索引以适应合并后的顶点数组
    unsigned int vertexOffset = static_cast<unsigned int>(combinedVertices.size());
    for (const auto &vertex : coneVertices)
    {
        Vertex newVertex = vertex;
        newVertex.Position += conePosition; // 将圆锥顶点位置偏移到
        combinedVertices.push_back(newVertex);
    }
    for (const auto &index : coneIndices)
    {
        combinedIndices.push_back(index + vertexOffset);
    }

    return std::make_tuple(combinedVertices, combinedIndices);
}

std::shared_ptr<Mesh> Geometry::CreateArrow(float length, float headSize)
{
    auto [vertices, indices] = GenerateArrowData(length, headSize);

    return std::make_shared<Mesh>(vertices, indices);
}

void Geometry::UpdateSphere(std::shared_ptr<Mesh> mesh, float radius, int segments)
{
    auto [vertices, indices] = GenerateSphereData(radius, segments);

    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateCube(std::shared_ptr<Mesh> mesh, float width, float height, float depth)
{
    auto [vertices, indices] = GenerateCubeData(width, height, depth);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateCylinder(std::shared_ptr<Mesh> mesh, float radius, float height, int segments)
{
    auto [vertices, indices] = GenerateCylinderData(radius, height, segments);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateCone(std::shared_ptr<Mesh> mesh, float radius, float height, int segments)
{
    auto [vertices, indices] = GenerateConeData(radius, height, segments);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdatePrism(std::shared_ptr<Mesh> mesh, int sides, float radius, float height)
{
    auto [vertices, indices] = GeneratePrismData(sides, radius, height);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdatePyramid(std::shared_ptr<Mesh> mesh, int sides, float radius, float height)
{
    auto [vertices, indices] = GeneratePyramidData(sides, radius, height);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateTorus(std::shared_ptr<Mesh> mesh, float majorRadius, float minorRadius, int majorSegments,
                           int minorSegments)
{
    auto [vertices, indices] = GenerateTorusData(majorRadius, minorRadius, majorSegments, minorSegments);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateEllipsoid(std::shared_ptr<Mesh> mesh, float radiusX, float radiusY, float radiusZ, int segments)
{
    auto [vertices, indices] = GenerateEllipsoidData(radiusX, radiusY, radiusZ, segments);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateFrustum(std::shared_ptr<Mesh> mesh, float radiusTop, float radiusBottom, float height,
                             int segments)
{
    auto [vertices, indices] = GenerateFrustumData(radiusTop, radiusBottom, height, segments);
    mesh->UpdateMesh(vertices, indices);
}

void Geometry::UpdateArrow(std::shared_ptr<Mesh> mesh, float length, float headSize)
{
    auto [vertices, indices] = GenerateArrowData(length, headSize);
    mesh->UpdateMesh(vertices, indices);
}