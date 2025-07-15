#include "core/Model.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

Model::Model(const std::string &path)
{
    LoadModel(path);
}

void Model::Draw(Shader &shader)
{
    for (auto &mesh : meshes)
    {
        mesh->Draw(shader);
    }
}

void Model::LoadModel(const std::string &path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode *node, const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;

    // 处理顶点
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // 位置
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        // 法线
        if (mesh->HasNormals())
        {
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        // 纹理坐标
        if (mesh->mTextureCoords[0])
        {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        // 切线
        if (mesh->HasTangentsAndBitangents())
        {
            vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);

            vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }

        vertices.push_back(vertex);
    }

    // 处理索引
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];
        material = LoadMaterial(aiMat);
    }
    else
    {
        material = std::make_shared<Material>();
    }

    return std::make_shared<Mesh>(vertices, indices, material);
}

std::shared_ptr<Material> Model::LoadMaterial(aiMaterial *mat)
{
    auto material = std::make_shared<Material>();
    aiString name;
    mat->Get(AI_MATKEY_NAME, name);
    material->name = name.C_Str();

    // 加载漫反射贴图
    std::vector<std::shared_ptr<Texture>> diffuseMaps =
        LoadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
    if (!diffuseMaps.empty())
    {
        material->diffuseMap = diffuseMaps[0];
    }

    // 加载镜面反射贴图
    std::vector<std::shared_ptr<Texture>> specularMaps =
        LoadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
    if (!specularMaps.empty())
    {
        material->specularMap = specularMaps[0];
    }

    // 加载法线贴图
    std::vector<std::shared_ptr<Texture>> normalMaps =
        LoadMaterialTextures(mat, aiTextureType_NORMALS, "texture_normal");
    if (!normalMaps.empty())
    {
        material->normalMap = normalMaps[0];
    }

    // 加载高度贴图
    std::vector<std::shared_ptr<Texture>> heightMaps =
        LoadMaterialTextures(mat, aiTextureType_HEIGHT, "texture_height");
    if (!heightMaps.empty())
    {
        // 视差贴图可以使用高度贴图
    }

    // 加载金属度/粗糙度贴图 (PBR)
    aiColor3D color;
    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
    {
        material->albedo = glm::vec3(color.r, color.g, color.b);
    }

    float metallic, roughness;
    if (mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
    {
        material->metallic = metallic;
    }

    if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
    {
        material->roughness = roughness;
    }

    return material;
}

std::vector<std::shared_ptr<Texture>> Model::LoadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                                                  const std::string &typeName)
{
    std::vector<std::shared_ptr<Texture>> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (auto &tex : texturesLoaded)
        {
            if (std::strcmp(tex->GetPath().data(), str.C_Str()) == 0)
            {
                textures.push_back(tex);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            auto texture = std::make_shared<Texture>();
            std::string filename = std::string(str.C_Str());
            filename = directory + '/' + filename;

            if (texture->LoadFromFile(filename))
            {
                texture->type = typeName;
                textures.push_back(texture);
                texturesLoaded.push_back(texture);
            }
        }
    }

    return textures;
}