#pragma once

#include "Mesh.hpp"
#include <assimp/scene.h>
#include <memory>
#include <string>
#include <vector>


class Model
{
  public:
    Model(const std::string &path);

    void Draw(Shader &shader);
    void DrawWithMaterialType(Shader &shader, MaterialType materialType);
    
    const std::vector<std::shared_ptr<Mesh>> &GetMeshes() const
    {
        return meshes;
    }
    const std::string &GetName() const
    {
        return name;
    }

    void SetTransform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scl)
    {
        position = pos;
        rotation = rot;
        scale = scl;
    }

    void SetName(const std::string &newName)
    {
        name = newName;
    }
    
    glm::vec3 GetPosition() const
    {
        return position;
    }
    glm::vec3 GetRotation() const
    {
        return rotation;
    }
    glm::vec3 GetScale() const
    {
        return scale;
    }
    const std::string &GetPath() const
    {
        return path;
    }

    // 计算模型的包围球（世界坐标系）
    // 返回 {center, radius}
    std::pair<glm::vec3, float> GetBoundingSphere() const;
    
    // 计算模型的AABB（局部坐标系）
    void GetLocalAABB(glm::vec3 &outMin, glm::vec3 &outMax) const;

  private:
    void LoadModel(const std::string &path);
    void ProcessNode(aiNode *node, const aiScene *scene);
    std::shared_ptr<Mesh> ProcessMesh(aiMesh *mesh, const aiScene *scene);
    std::shared_ptr<Material> LoadMaterial(aiMaterial *mat);
    std::vector<std::shared_ptr<Texture>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                                               const std::string &typeName);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::string directory;
    std::vector<std::shared_ptr<Texture>> texturesLoaded;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    std::string name = "Model";
    std::string path; // 模型文件路径
};