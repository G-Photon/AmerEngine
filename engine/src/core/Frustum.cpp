#include "core/Frustum.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

void Frustum::ExtractFromMatrices(const glm::mat4& view, const glm::mat4& projection)
{
    // 计算视锥矩阵（投影 * 视图）
    glm::mat4 vpMatrix = projection * view;

    // 提取六个平面（使用列向量）
    // 右平面：M3 - M0
    planes[RIGHT].normal = glm::vec3(vpMatrix[0][3] - vpMatrix[0][0],
                                     vpMatrix[1][3] - vpMatrix[1][0],
                                     vpMatrix[2][3] - vpMatrix[2][0]);
    planes[RIGHT].distance = vpMatrix[3][3] - vpMatrix[3][0];

    // 左平面：M3 + M0
    planes[LEFT].normal = glm::vec3(vpMatrix[0][3] + vpMatrix[0][0],
                                    vpMatrix[1][3] + vpMatrix[1][0],
                                    vpMatrix[2][3] + vpMatrix[2][0]);
    planes[LEFT].distance = vpMatrix[3][3] + vpMatrix[3][0];

    // 上平面：M3 - M1
    planes[TOP].normal = glm::vec3(vpMatrix[0][3] - vpMatrix[0][1],
                                   vpMatrix[1][3] - vpMatrix[1][1],
                                   vpMatrix[2][3] - vpMatrix[2][1]);
    planes[TOP].distance = vpMatrix[3][3] - vpMatrix[3][1];

    // 下平面：M3 + M1
    planes[BOTTOM].normal = glm::vec3(vpMatrix[0][3] + vpMatrix[0][1],
                                      vpMatrix[1][3] + vpMatrix[1][1],
                                      vpMatrix[2][3] + vpMatrix[2][1]);
    planes[BOTTOM].distance = vpMatrix[3][3] + vpMatrix[3][1];

    // 远平面：M3 - M2
    planes[FAR].normal = glm::vec3(vpMatrix[0][3] - vpMatrix[0][2],
                                   vpMatrix[1][3] - vpMatrix[1][2],
                                   vpMatrix[2][3] - vpMatrix[2][2]);
    planes[FAR].distance = vpMatrix[3][3] - vpMatrix[3][2];

    // 近平面：M3 + M2
    planes[NEAR].normal = glm::vec3(vpMatrix[0][3] + vpMatrix[0][2],
                                    vpMatrix[1][3] + vpMatrix[1][2],
                                    vpMatrix[2][3] + vpMatrix[2][2]);
    planes[NEAR].distance = vpMatrix[3][3] + vpMatrix[3][2];

    // 标准化所有平面
    for (auto& plane : planes)
    {
        NormalizePlane(plane);
    }
}

void Frustum::NormalizePlane(Plane& plane)
{
    float length = glm::length(plane.normal);
    if (length > 0.0f)
    {
        plane.normal /= length;
        plane.distance /= length;
    }
}

bool Frustum::IsSphereInFrustum(const glm::vec3& center, float radius) const
{
    for (const auto& plane : planes)
    {
        if (plane.GetDistance(center) < -radius)
        {
            return false; // 球完全在平面外侧
        }
    }
    return true; // 球在视锥体内或与之相交
}

bool Frustum::IsAABBInFrustum(const glm::vec3& min, const glm::vec3& max) const
{
    for (const auto& plane : planes)
    {
        // 计算AABB相对于平面的最近和最远点
        glm::vec3 nearPoint(
            plane.normal.x > 0 ? min.x : max.x,
            plane.normal.y > 0 ? min.y : max.y,
            plane.normal.z > 0 ? min.z : max.z
        );

        glm::vec3 farPoint(
            plane.normal.x > 0 ? max.x : min.x,
            plane.normal.y > 0 ? max.y : min.y,
            plane.normal.z > 0 ? max.z : min.z
        );

        // 如果最近点在平面外侧，AABB完全在视锥体外
        if (plane.GetDistance(nearPoint) < 0.0f)
        {
            return false;
        }
    }
    return true; // AABB在视锥体内或与之相交
}
