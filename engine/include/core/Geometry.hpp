#pragma once
#include "Mesh.hpp"
#include <glm/glm.hpp>
#include <memory>

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
};