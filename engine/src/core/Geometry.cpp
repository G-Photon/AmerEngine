#include "core/Geometry.hpp"
#include <glm/gtc/constants.hpp>
#include <vector>

std::wstring Geometry::name[Geometry::Type::END + 1] = {
    L"球体", L"立方体", L"圆柱体", L"圆锥体", L"棱柱", L"金字塔", L"环面", L"椭球体", L"截头锥"};

std::shared_ptr<Mesh> Geometry::CreateSphere(float radius, int segments)
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

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCube(float width, float height, float depth)
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

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCylinder(float radius, float height, int segments)
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
    return std::make_shared<Mesh>(vertices, indices);
}
std::shared_ptr<Mesh> Geometry::CreateCone(float radius, float height, int segments)
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

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreatePrism(int sides, float radius, float height)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;
    float angleStep = 2.0f * glm::pi<float>() / sides;

    // // 顶部中心顶点
    // vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0.5f}});
    // // 底部中心顶点
    // vertices.push_back({{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}});
    // 生成顶部、底部和侧面顶点
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

    return std::make_shared<Mesh>(vertices, indices);
}
std::shared_ptr<Mesh> Geometry::CreatePyramid(int sides, float baseSize, float height)
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

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments)
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

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateEllipsoid(float radiusX, float radiusY, float radiusZ, int segments)
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

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateFrustum(float radiusTop, float radiusBottom, float height, int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;

    // 顶部中心顶点
    vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0.5f}});
    // 底部中心顶点
    vertices.push_back({{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}});

    // 生成顶部和底部圆面顶点
    for (int i = 0; i <= segments; ++i) // 注意：i <= segments 确保闭合
    {
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float xTop = radiusTop * cos(theta);
        float zTop = radiusTop * sin(theta);
        float xBottom = radiusBottom * cos(theta);
        float zBottom = radiusBottom * sin(theta);

        // 顶部圆面顶点（法线朝上）
        vertices.push_back({{xTop, halfHeight, zTop}, {0, 1, 0}, {i / (float)segments, 0}});
        // 底部圆面顶点（法线朝下）
        vertices.push_back({{xBottom, -halfHeight, zBottom}, {0, -1, 0}, {i / (float)segments, 1}});
    }

    // 生成侧面顶点
    for (int i = 0; i < segments; ++i)
    {
        int topIndex = 2 + i * 2;          // 顶部圆面顶点索引
        int bottomIndex = topIndex + 1;   // 底部圆面顶点索引
        int nextTopIndex = topIndex + 2;   // 下一个顶部圆面顶点索引
        int nextBottomIndex = bottomIndex + 2; // 下一个底部圆面顶点索引

        // 计算当前和下一个的圆面顶点坐标
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float xTop = radiusTop * cos(theta);
        float zTop = radiusTop * sin(theta);
        float xBottom = radiusBottom * cos(theta);
        float zBottom = radiusBottom * sin(theta);

        glm::vec3 sideNormal((radiusTop - radiusBottom) / height,
                             (radiusTop + radiusBottom) / (2.0f * height), 0);
        sideNormal = glm::normalize(sideNormal);

        // 侧面顶点
        vertices.push_back({{xTop, halfHeight, zTop}, sideNormal, {i / (float)segments, 0}});
        vertices.push_back({{xBottom, -halfHeight, zBottom}, sideNormal, {i / (float)segments, 1}});

        // 生成侧面索引
        if (i < segments - 1)
        {
            indices.push_back(topIndex);
            indices.push_back(nextTopIndex);
            indices.push_back(bottomIndex);

            indices.push_back(bottomIndex);
            indices.push_back(nextTopIndex);
            indices.push_back(nextBottomIndex);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}