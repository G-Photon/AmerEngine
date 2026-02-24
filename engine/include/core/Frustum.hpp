#pragma once

#include <glm/glm.hpp>
#include <array>

/**
 * @class Frustum
 * @brief 视锥体类，用于视锥剔除
 * 
 * 该类表示摄像机的视锥体，可以与包围盒 (AABB) 和球体进行相交测试
 * 用于判断场景对象是否在摄像机视野范围内
 */
class Frustum
{
public:
    enum PlaneID
    {
        NEAR = 0,
        FAR = 1,
        LEFT = 2,
        RIGHT = 3,
        TOP = 4,
        BOTTOM = 5,
        PLANE_COUNT = 6
    };

    struct Plane
    {
        glm::vec3 normal;
        float distance;

        Plane() : normal(0.0f), distance(0.0f) {}
        Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}

        /**
         * @brief 计算点到平面的距离
         * @param point 点坐标
         * @return 如果为正则点在平面法线指向的一侧，负则在另一侧
         */
        float GetDistance(const glm::vec3& point) const
        {
            return glm::dot(normal, point) + distance;
        }
    };

    Frustum() = default;

    /**
     * @brief 从视图矩阵和投影矩阵提取视锥体
     * @param view 视图矩阵
     * @param projection 投影矩阵
     */
    void ExtractFromMatrices(const glm::mat4& view, const glm::mat4& projection);

    /**
     * @brief 判断球体是否与视锥体相交
     * @param center 球心
     * @param radius 球半径
     * @return 球体是否在视锥体内或与之相交
     */
    bool IsSphereInFrustum(const glm::vec3& center, float radius) const;

    /**
     * @brief 判断AABB是否与视锥体相交
     * @param min AABB最小点
     * @param max AABB最大点
     * @return AABB是否在视锥体内或与之相交
     */
    bool IsAABBInFrustum(const glm::vec3& min, const glm::vec3& max) const;

    /**
     * @brief 获取指定平面
     * @param id 平面ID
     * @return 对应的平面
     */
    const Plane& GetPlane(PlaneID id) const { return planes[id]; }

private:
    std::array<Plane, PLANE_COUNT> planes;

    /**
     * @brief 标准化平面
     * @param plane 待标准化的平面
     */
    void NormalizePlane(Plane& plane);
};
