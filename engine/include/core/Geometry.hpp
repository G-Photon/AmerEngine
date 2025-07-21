#pragma once
#include "Mesh.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <tuple>

class Geometry
{
  public:
    enum Type
    {
        SPHERE,
        CUBE,
        CYLINDER,
        CONE,
        PRISM,
        PYRAMID,
        TORUS,
        ELLIPSOID,
        FRUSTUM,
        ARROW,
        END
    };

    static std::wstring name[Geometry::Type::END + 1];
    struct Primitive
    {
        Type type;
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
        std::shared_ptr<Mesh> mesh;
        union {
            struct
            {
                float radius;
                int segments;
            } sphere;
            struct
            {
                float width, height, depth;
            } cube;
            struct
            {
                float radius, height;
                int segments;
            } cylinder;
            // ... 其他几何体参数
            struct
            {
                float radius, height;
                int segments;
            } cone;
            struct
            {
                int sides;
                float radius, height;
            } prism;
            struct
            {
                int sides;
                float radius, height;
            } pyramid;
            struct
            {
                float majorRadius, minorRadius;
                int majorSegments, minorSegments;
            } torus;
            struct
            {
                float radiusX, radiusY, radiusZ;
                int segments;
            } ellipsoid;
            struct
            {
                float radiusTop, radiusBottom, height;
                int segments;
            } frustum;
            struct
            {
                float length, headSize;
            } arrow;
        } params;

        void SetTransform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scl)
        {
            position = pos;
            rotation = rot;
            scale = scl;
            if (mesh)
            {
                mesh->SetTransform(pos, rot, scl);
            }
        }
    };

    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateSphereData(float radius, int segments);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateCubeData(float width, float height,
                                                                                       float depth);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateCylinderData(float radius, float height,
                                                                                           int segments);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateConeData(float radius, float height,
                                                                                       int segments);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GeneratePrismData(int sides, float radius,
                                                                                        float height);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GeneratePyramidData(int sides, float radius,
                                                                                          float height);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateTorusData(float majorRadius,
                                                                                        float minorRadius,
                                                                                        int majorSegments,
                                                                                        int minorSegments);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateEllipsoidData(float radiusX,
                                                                                            float radiusY,
                                                                                            float radiusZ,
                                                                                            int segments);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateFrustumData(float radiusTop,

                                                                                          float radiusBottom,
                                                                                          float height, int segments);
    static std::tuple<std::vector<Vertex>, std::vector<unsigned int>> GenerateArrowData(float length, float headSize);

    static std::shared_ptr<Mesh> CreateSphere(float radius = 1.0f, int segments = 32);
    static std::shared_ptr<Mesh> CreateCube(float width = 1.0f, float height = 1.0f, float depth = 1.0f);
    static std::shared_ptr<Mesh> CreateCylinder(float radius = 0.5f, float height = 1.0f, int segments = 32);
    static std::shared_ptr<Mesh> CreateCone(float radius = 0.5f, float height = 1.0f, int segments = 32);
    static std::shared_ptr<Mesh> CreatePrism(int sides = 6, float radius = 0.5f, float height = 1.0f);
    static std::shared_ptr<Mesh> CreatePyramid(int sides = 4, float radius = 0.5f, float height = 1.0f);
    static std::shared_ptr<Mesh> CreateTorus(float majorRadius = 1.0f, float minorRadius = 0.3f, int majorSegments = 48,
                                             int minorSegments = 12);
    static std::shared_ptr<Mesh> CreateEllipsoid(float radiusX = 1.0f, float radiusY = 0.8f, float radiusZ = 1.2f,
                                                 int segments = 32);
    static std::shared_ptr<Mesh> CreateFrustum(float radiusTop = 0.5f, float radiusBottom = 0.5f, float height = 1.0f,
                                               int segments = 32);
    static std::shared_ptr<Mesh> CreateArrow(float length, float headSize);

    static void UpdateSphere(std::shared_ptr<Mesh> mesh, float radius, int segments);
    static void UpdateCube(std::shared_ptr<Mesh> mesh, float width, float height, float depth);
    static void UpdateCylinder(std::shared_ptr<Mesh> mesh, float radius, float height, int segments);
    static void UpdateCone(std::shared_ptr<Mesh> mesh, float radius, float height, int segments);
    static void UpdatePrism(std::shared_ptr<Mesh> mesh, int sides, float radius, float height);
    static void UpdatePyramid(std::shared_ptr<Mesh> mesh, int sides, float radius, float height);
    static void UpdateTorus(std::shared_ptr<Mesh> mesh, float majorRadius, float minorRadius, int majorSegments,
                            int minorSegments);
    static void UpdateEllipsoid(std::shared_ptr<Mesh> mesh, float radiusX, float radiusY, float radiusZ, int segments);
    static void UpdateFrustum(std::shared_ptr<Mesh> mesh, float radiusTop, float radiusBottom, float height,
                              int segments);
    static void UpdateArrow(std::shared_ptr<Mesh> mesh, float length, float headSize);
};