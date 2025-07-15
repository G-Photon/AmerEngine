#include "core/Geometry.hpp"
#include <glm/gtc/constants.hpp>
#include <vector>


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
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCube(float width, float height, float depth)
{
    std::vector<Vertex> vertices = {
        // 前面
        {{-width, -height, depth}, {0, 0, 1}, {0, 0}},
        {{width, -height, depth}, {0, 0, 1}, {1, 0}},
        {{width, height, depth}, {0, 0, 1}, {1, 1}},
        {{-width, height, depth}, {0, 0, 1}, {0, 1}},

        // 后面
        {{-width, -height, -depth}, {0, 0, -1}, {1, 0}},
        {{width, -height, -depth}, {0, 0, -1}, {0, 0}},
        {{width, height, -depth}, {0, 0, -1}, {0, 1}},
        {{-width, height, -depth}, {0, 0, -1}, {1, 1}},

        // 左面
        {{-width, -height, -depth}, {-1, 0, 0}, {0, 0}},
        {{-width, -height, depth}, {-1, 0, 0}, {1, 0}},
        {{-width, height, depth}, {-1, 0, 0}, {1, 1}},
        {{-width, height, -depth}, {-1, 0, 0}, {0, 1}},

        // 右面
        {{width, -height, -depth}, {1, 0, 0}, {1, 0}},
        {{width, -height, depth}, {1, 0, 0}, {0, 0}},
        {{width, height, depth}, {1, 0, 0}, {0, 1}},
        {{width, height, -depth}, {1, 0, 0}, {1, 1}},

        // 顶部
        {{-width, height, -depth}, {0, 1, 0}, {0, 1}},
        {{width, height, -depth}, {0, 1, 0}, {1, 1}},
        {{width, height, depth}, {0, 1, 0}, {1, 0}},
        {{-width, height, depth}, {0, 1, 0}, {0, 0}},

        // 底部
        {{-width, -height, -depth}, {0, -1, 0}, {0, 0}},
        {{width, -height, -depth}, {0, -1, 0}, {1, 0}},
        {{width, -height, depth}, {0, -1, 0}, {1, 1}},
        {{-width, -height, depth}, {0, -1, 0}, {0, 1}}
    };

    std::vector<unsigned int> indices = {
        // 前面
        0, 1, 2, 2, 3, 0,
        // 后面
        4, 5, 6, 6, 7, 4,
        // 左面
        8, 9, 10, 10, 11, 8,
        // 右面
        12, 13, 14, 14, 15, 12,
        // 顶部
        16, 17, 18, 18, 19, 16,
        // 底部
        20, 21, 22, 22, 23, 20
    };

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

        // 顶部顶点
        vertices.push_back({{x, halfHeight, z}, {0, 1, 0}, {i / (float)segments, 0}});
        // 底部顶点
        vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {i / (float)segments, 1}});
    }

    // 生成索引
    for (int i = 0; i < segments; ++i)
    {
        int top1 = i * 2;
        int top2 = (i + 1) * 2;
        int bottom1 = i * 2 + 1;
        int bottom2 = (i + 1) * 2 + 1;

        // 上半部分
        indices.push_back(top1);
        indices.push_back(top2);
        indices.push_back(bottom1);

        indices.push_back(top2);
        indices.push_back(bottom2);
        indices.push_back(bottom1);
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateCone(float radius, float height, int segments)
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

        // 底部顶点
        vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {i / (float)segments, 1}});
    }

    // 顶部顶点
    vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0}});

    // 生成索引
    for (int i = 0; i < segments; ++i)
    {
        int bottom1 = i;
        int bottom2 = (i + 1) % segments;
        int top = segments;

        // 上半部分
        indices.push_back(bottom1);
        indices.push_back(bottom2);
        indices.push_back(top);
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreatePrism(int sides, float radius, float height)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;
    float angleStep = 2.0f * glm::pi<float>() / sides;

    // 生成顶点
    for (int i = 0; i < sides; ++i)
    {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // 顶部顶点
        vertices.push_back({{x, halfHeight, z}, {0, 1, 0}, {i / (float)sides, 0}});
        // 底部顶点
        vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {i / (float)sides, 1}});
    }

    // 生成索引
    for (int i = 0; i < sides; ++i)
    {
        int top1 = i * 2;
        int top2 = (i + 1) % sides * 2;
        int bottom1 = i * 2 + 1;
        int bottom2 = (i + 1) % sides * 2 + 1;

        // 上半部分
        indices.push_back(top1);
        indices.push_back(top2);
        indices.push_back(bottom1);

        indices.push_back(top2);
        indices.push_back(bottom2);
        indices.push_back(bottom1);
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreatePyramid(int sides, float baseSize, float height)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfBase = baseSize / 2.0f;

    // 生成底部顶点
    for (int i = 0; i < sides; ++i)
    {
        float angle = i * 2.0f * glm::pi<float>() / sides;
        float x = halfBase * cos(angle);
        float z = halfBase * sin(angle);

        vertices.push_back({{x, 0, z}, {0, -1, 0}, {i / (float)sides, 1}});
    }

    // 生成顶部顶点
    vertices.push_back({{0, height, 0}, {0, 1, 0}, {0.5f, 0}});

    // 生成索引
    for (int i = 0; i < sides; ++i)
    {
        int bottom1 = i;
        int bottom2 = (i + 1) % sides;
        int top = sides;

        // 上半部分
        indices.push_back(bottom1);
        indices.push_back(bottom2);
        indices.push_back(top);
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= majorSegments; ++i)
    {
        float theta = i * 2.0f * glm::pi<float>() / majorSegments;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        for (int j = 0; j <= minorSegments; ++j)
        {
            float phi = j * 2.0f * glm::pi<float>() / minorSegments;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);

            float x = (majorRadius + minorRadius * cosPhi) * cosTheta;
            float y = minorRadius * sinPhi;
            float z = (majorRadius + minorRadius * cosPhi) * sinTheta;

            glm::vec3 position(x, y, z);
            glm::vec3 normal(cosPhi * cosTheta, sinPhi, cosPhi * sinTheta);
            glm::vec2 texCoord((float)i / majorSegments, (float)j / minorSegments);

            vertices.push_back({position, normal, texCoord});
        }
    }

    for (int i = 0; i < majorSegments; ++i)
    {
        for (int j = 0; j < minorSegments; ++j)
        {
            int first = i * (minorSegments + 1) + j;
            int second = first + minorSegments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Geometry::CreateEllipsoid(float radiusX, float radiusY, float radiusZ, int segments)
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

            float x = radiusX * sin(phi) * cos(theta);
            float y = radiusY * cos(phi);
            float z = radiusZ * sin(phi) * sin(theta);

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
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}