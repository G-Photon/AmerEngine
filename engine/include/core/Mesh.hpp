#pragma once

#include "Material.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <vector>


struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

class Mesh
{
  public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices,
         const std::shared_ptr<Material> &material = nullptr);
    ~Mesh();

    void Draw(Shader &shader);

    const std::vector<Vertex> &GetVertices() const
    {
        return vertices;
    }
    const std::vector<unsigned int> &GetIndices() const
    {
        return indices;
    }
    const std::shared_ptr<Material> &GetMaterial() const
    {
        return material;
    }

    void SetMaterial(const std::shared_ptr<Material> &mat)
    {
        material = mat;
    }
    void SetTransform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scl)
    {
        position = pos;
        rotation = rot;
        scale = scl;
    }

    const std::string &GetName() const
    {
        return name;
    }
    void SetName(const std::string &n)
    {
        name = n;
    }

    void UpdateMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
    {
        // 删除旧数据
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        // 重新设置数据
        this->vertices = vertices;
        this->indices = indices;
        // 重新创建VAO和VBO
        SetupMesh();
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
  private:
    void SetupMesh();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;

    unsigned int VAO, VBO, EBO;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    std::string name = "Mesh";
};