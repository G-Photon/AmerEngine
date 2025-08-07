// 核心渲染器定义
#include "core/Renderer.hpp"
#include "core/Camera.hpp"
#include "core/Framebuffer.hpp"
// 序列化
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// 日志与随机
#include <iostream>
#include <random>
// 文件系统辅助
#include "utils/FileSystem.hpp"
// 纹理类型
#include "core/Texture.hpp"
#include "core/TextureManager.hpp"
// 数学运算
#include "glm/gtc/quaternion.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/matrix_transform.hpp>
// STB图像处理（仅头文件，实现在Texture.cpp中）
#include "stb_image.h"
// 新建场景：清空所有模型、光源、几何体等
void Renderer::NewScene()
{
    models.clear();
    pointLights.clear();
    directionalLights.clear();
    spotLights.clear();
    primitives.clear();
    // 可根据需要重置相机等
}

void Renderer::SaveScene(const std::string& path)
{
    json j;
    
    // 保存场景基本信息
    j["sceneInfo"] = {
        {"name", "Scene"},
        {"version", "1.0"},
        {"createdTime", std::time(nullptr)}
    };
    
    // 保存模型
    j["models"] = json::array();
    for (const auto& model : models) {
        json modelJson = {
            {"name", model->GetName()},
            {"path", model->GetPath()},
            {"position", {model->GetPosition().x, model->GetPosition().y, model->GetPosition().z}},
            {"rotation", {model->GetRotation().x, model->GetRotation().y, model->GetRotation().z}},
            {"scale", {model->GetScale().x, model->GetScale().y, model->GetScale().z}}
        };
        
        // 保存模型的材质信息
        modelJson["materials"] = json::array();
        const auto& meshes = model->GetMeshes();
        for (size_t i = 0; i < meshes.size(); ++i) {
            const auto& materialPtr = meshes[i]->GetMaterial();
            if (materialPtr) {
                const auto& material = *materialPtr;
                json materialJson = {
                    {"meshIndex", i},
                    {"type", static_cast<int>(material.type)},
                    {"albedo", {material.albedo.r, material.albedo.g, material.albedo.b}},
                    {"metallic", material.metallic},
                    {"roughness", material.roughness},
                    {"ao", material.ao},
                    {"diffuse", {material.diffuse.r, material.diffuse.g, material.diffuse.b}},
                    {"specular", {material.specular.r, material.specular.g, material.specular.b}},
                    {"shininess", material.shininess},
                    {"useAlbedoMap", material.useAlbedoMap},
                    {"useMetallicMap", material.useMetallicMap},
                    {"useRoughnessMap", material.useRoughnessMap},
                    {"useNormalMap", material.useNormalMap},
                    {"useAOMap", material.useAOMap},
                    {"useDiffuseMap", material.useDiffuseMap},
                    {"useSpecularMap", material.useSpecularMap}
                };
                
                // 保存贴图路径和flipY状态
                if (material.albedoMap) {
                    materialJson["albedoMapPath"] = material.albedoMap->GetPath();
                    materialJson["albedoMapFlipY"] = material.albedoMap->flipY;
                }
                if (material.metallicMap) {
                    materialJson["metallicMapPath"] = material.metallicMap->GetPath();
                    materialJson["metallicMapFlipY"] = material.metallicMap->flipY;
                }
                if (material.roughnessMap) {
                    materialJson["roughnessMapPath"] = material.roughnessMap->GetPath();
                    materialJson["roughnessMapFlipY"] = material.roughnessMap->flipY;
                }
                if (material.normalMap) {
                    materialJson["normalMapPath"] = material.normalMap->GetPath();
                    materialJson["normalMapFlipY"] = material.normalMap->flipY;
                }
                if (material.aoMap) {
                    materialJson["aoMapPath"] = material.aoMap->GetPath();
                    materialJson["aoMapFlipY"] = material.aoMap->flipY;
                }
                if (material.diffuseMap) {
                    materialJson["diffuseMapPath"] = material.diffuseMap->GetPath();
                    materialJson["diffuseMapFlipY"] = material.diffuseMap->flipY;
                }
                if (material.specularMap) {
                    materialJson["specularMapPath"] = material.specularMap->GetPath();
                    materialJson["specularMapFlipY"] = material.specularMap->flipY;
                }
                
                modelJson["materials"].push_back(materialJson);
            }
        }
        
        j["models"].push_back(modelJson);
    }
    
    // 保存点光源
    j["pointLights"] = json::array();
    for (const auto& light : pointLights) {
        j["pointLights"].push_back({
            {"position", {light->position.x, light->position.y, light->position.z}},
            {"diffuse", {light->diffuse.r, light->diffuse.g, light->diffuse.b}},
            {"specular", {light->specular.r, light->specular.g, light->specular.b}},
            {"ambient", {light->ambient.r, light->ambient.g, light->ambient.b}},
            {"intensity", light->intensity},
            {"constant", light->constant},
            {"linear", light->linear},
            {"quadratic", light->quadratic},
            {"shadowEnabled", light->HasShadows()}
        });
    }
    
    // 保存定向光源
    j["directionalLights"] = json::array();
    for (const auto& light : directionalLights) {
        j["directionalLights"].push_back({
            {"direction", {light->direction.x, light->direction.y, light->direction.z}},
            {"diffuse", {light->diffuse.r, light->diffuse.g, light->diffuse.b}},
            {"specular", {light->specular.r, light->specular.g, light->specular.b}},
            {"ambient", {light->ambient.r, light->ambient.g, light->ambient.b}},
            {"intensity", light->intensity},
            {"shadowEnabled", light->HasShadows()}
        });
    }
    
    // 保存聚光灯
    j["spotLights"] = json::array();
    for (const auto& light : spotLights) {
        j["spotLights"].push_back({
            {"position", {light->position.x, light->position.y, light->position.z}},
            {"direction", {light->direction.x, light->direction.y, light->direction.z}},
            {"diffuse", {light->diffuse.r, light->diffuse.g, light->diffuse.b}},
            {"specular", {light->specular.r, light->specular.g, light->specular.b}},
            {"ambient", {light->ambient.r, light->ambient.g, light->ambient.b}},
            {"intensity", light->intensity},
            {"constant", light->constant},
            {"linear", light->linear},
            {"quadratic", light->quadratic},
            {"cutOff", light->cutOff},
            {"outerCutOff", light->outerCutOff},
            {"shadowEnabled", light->HasShadows()}
        });
    }
    
    // 保存几何体
    j["primitives"] = json::array();
    for (const auto& primitive : primitives) {
        json primitiveJson = {
            {"type", static_cast<int>(primitive.type)},
            {"position", {primitive.position.x, primitive.position.y, primitive.position.z}},
            {"rotation", {primitive.rotation.x, primitive.rotation.y, primitive.rotation.z}},
            {"scale", {primitive.scale.x, primitive.scale.y, primitive.scale.z}},
            {"material", {
                {"type", static_cast<int>(primitive.mesh->GetMaterial()->type)},
                {"diffuse", {primitive.mesh->GetMaterial()->diffuse.r, primitive.mesh->GetMaterial()->diffuse.g, primitive.mesh->GetMaterial()->diffuse.b}},
                {"specular", {primitive.mesh->GetMaterial()->specular.r, primitive.mesh->GetMaterial()->specular.g, primitive.mesh->GetMaterial()->specular.b}},
                {"shininess", primitive.mesh->GetMaterial()->shininess},
                {"useDiffuseMap", primitive.mesh->GetMaterial()->useDiffuseMap},
                {"useSpecularMap", primitive.mesh->GetMaterial()->useSpecularMap},
                {"useNormalMap", primitive.mesh->GetMaterial()->useNormalMap},
                {"diffuseMapPath", primitive.mesh->GetMaterial()->diffuseMap ? primitive.mesh->GetMaterial()->diffuseMap->GetPath() : ""},
                {"specularMapPath", primitive.mesh->GetMaterial()->specularMap ? primitive.mesh->GetMaterial()->specularMap->GetPath() : ""},
                {"normalMapPath", primitive.mesh->GetMaterial()->normalMap ? primitive.mesh->GetMaterial()->normalMap->GetPath() : ""},
                {"diffuseMapFlipY", primitive.mesh->GetMaterial()->diffuseMap ? primitive.mesh->GetMaterial()->diffuseMap->flipY : true},
                {"specularMapFlipY", primitive.mesh->GetMaterial()->specularMap ? primitive.mesh->GetMaterial()->specularMap->flipY : true},
                {"normalMapFlipY", primitive.mesh->GetMaterial()->normalMap ? primitive.mesh->GetMaterial()->normalMap->flipY : true}
            }}
        };
        
        // 保存几何体参数
        switch (primitive.type) {
            case Geometry::SPHERE:
                primitiveJson["params"] = {
                    {"radius", primitive.params.sphere.radius},
                    {"segments", primitive.params.sphere.segments}
                };
                break;
            case Geometry::CUBE:
                primitiveJson["params"] = {
                    {"width", primitive.params.cube.width},
                    {"height", primitive.params.cube.height},
                    {"depth", primitive.params.cube.depth}
                };
                break;
            case Geometry::CYLINDER:
                primitiveJson["params"] = {
                    {"radius", primitive.params.cylinder.radius},
                    {"height", primitive.params.cylinder.height},
                    {"segments", primitive.params.cylinder.segments}
                };
                break;
            case Geometry::CONE:
                primitiveJson["params"] = {
                    {"radius", primitive.params.cone.radius},
                    {"height", primitive.params.cone.height},
                    {"segments", primitive.params.cone.segments}
                };
                break;
            case Geometry::PRISM:
                primitiveJson["params"] = {
                    {"sides", primitive.params.prism.sides},
                    {"radius", primitive.params.prism.radius},
                    {"height", primitive.params.prism.height}
                };
                break;
            case Geometry::PYRAMID:
                primitiveJson["params"] = {
                    {"sides", primitive.params.pyramid.sides},
                    {"radius", primitive.params.pyramid.radius},
                    {"height", primitive.params.pyramid.height}
                };
                break;
            case Geometry::TORUS:
                primitiveJson["params"] = {
                    {"majorRadius", primitive.params.torus.majorRadius},
                    {"minorRadius", primitive.params.torus.minorRadius},
                    {"majorSegments", primitive.params.torus.majorSegments},
                    {"minorSegments", primitive.params.torus.minorSegments}
                };
                break;
            case Geometry::ELLIPSOID:
                primitiveJson["params"] = {
                    {"radiusX", primitive.params.ellipsoid.radiusX},
                    {"radiusY", primitive.params.ellipsoid.radiusY},
                    {"radiusZ", primitive.params.ellipsoid.radiusZ},
                    {"segments", primitive.params.ellipsoid.segments}
                };
                break;
            case Geometry::FRUSTUM:
                primitiveJson["params"] = {
                    {"radiusTop", primitive.params.frustum.radiusTop},
                    {"radiusBottom", primitive.params.frustum.radiusBottom},
                    {"height", primitive.params.frustum.height},
                    {"segments", primitive.params.frustum.segments}
                };
                break;
            case Geometry::ARROW:
                primitiveJson["params"] = {
                    {"length", primitive.params.arrow.length},
                    {"headSize", primitive.params.arrow.headSize}
                };
                break;
            default:
                // 未知类型，使用默认参数
                primitiveJson["params"] = {};
                break;
        }
        
        j["primitives"].push_back(primitiveJson);
    }
    
    // 保存相机状态
    if (mainCamera) {
        j["camera"] = {
            {"position", {mainCamera->Position.x, mainCamera->Position.y, mainCamera->Position.z}},
            {"front", {mainCamera->Front.x, mainCamera->Front.y, mainCamera->Front.z}},
            {"up", {mainCamera->Up.x, mainCamera->Up.y, mainCamera->Up.z}},
            {"yaw", mainCamera->Yaw},
            {"pitch", mainCamera->Pitch},
            {"fov", mainCamera->Zoom},
            {"movementSpeed", mainCamera->MovementSpeed},
            {"mouseSensitivity", mainCamera->MouseSensitivity}
        };
    }
    
    // 保存渲染设置
    j["renderSettings"] = {
        {"renderMode", static_cast<int>(currentMode)},
        {"shadowEnabled", shadowEnabled},
        {"hdrEnabled", hdrEnabled},
        {"bloomEnabled", bloomEnabled},
        {"ssaoEnabled", ssaoEnabled},
        {"msaaEnabled", msaaEnabled},
        {"fxaaEnabled", fxaaEnabled},
        {"gammaCorrection", gammaCorrection},
        {"iblEnabled", iblEnabled},
        {"showLights", showLights},
        {"backgroundType", static_cast<int>(backgroundType)}
    };
    
    std::ofstream ofs(path);
    if (ofs.is_open()) {
        ofs << j.dump(4);
        std::cout << "场景已保存到: " << path << std::endl;
    } else {
        std::cerr << "无法保存场景文件: " << path << std::endl;
    }
}

#include <ctime>

// 从文件加载场景
void Renderer::LoadScene(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "无法打开场景文件: " << path << std::endl;
        return;
    }
    
    json j;
    try {
        ifs >> j;
    } catch (const json::exception& e) {
        std::cerr << "场景文件格式错误: " << e.what() << std::endl;
        return;
    }
    
    NewScene(); // 清空当前场景
    
    // 加载模型
    if (j.contains("models")) {
        for (const auto& m : j["models"]) {
            try {
                auto model = std::make_shared<Model>(m["path"].get<std::string>());
                if (m.contains("name")) {
                    model->SetName(m["name"].get<std::string>());
                }
                auto pos = m["position"];
                auto rot = m["rotation"];
                auto scl = m["scale"];
                model->SetTransform(
                    glm::vec3(pos[0], pos[1], pos[2]),
                    glm::vec3(rot[0], rot[1], rot[2]),
                    glm::vec3(scl[0], scl[1], scl[2])
                );
                
                // 加载材质信息
                if (m.contains("materials")) {
                    const auto& meshes = model->GetMeshes();
                    for (const auto& materialData : m["materials"]) {
                        try {
                            size_t meshIndex = materialData["meshIndex"].get<size_t>();
                            if (meshIndex < meshes.size()) {
                                auto materialPtr = meshes[meshIndex]->GetMaterial();
                                if (materialPtr) {
                                    // 更新材质属性
                                    if (materialData.contains("type")) {
                                        materialPtr->type = static_cast<MaterialType>(materialData["type"].get<int>());
                                    }
                                    if (materialData.contains("albedo")) {
                                        auto c = materialData["albedo"];
                                        materialPtr->albedo = glm::vec3(c[0], c[1], c[2]);
                                    }
                                    if (materialData.contains("metallic")) {
                                        materialPtr->metallic = materialData["metallic"];
                                    }
                                    if (materialData.contains("roughness")) {
                                        materialPtr->roughness = materialData["roughness"];
                                    }
                                    if (materialData.contains("ao")) {
                                        materialPtr->ao = materialData["ao"];
                                    }
                                    if (materialData.contains("diffuse")) {
                                        auto c = materialData["diffuse"];
                                        materialPtr->diffuse = glm::vec3(c[0], c[1], c[2]);
                                    }
                                    if (materialData.contains("specular")) {
                                        auto c = materialData["specular"];
                                        materialPtr->specular = glm::vec3(c[0], c[1], c[2]);
                                    }
                                    if (materialData.contains("shininess")) {
                                        materialPtr->shininess = materialData["shininess"];
                                    }
                                    
                                    // 更新贴图使用标志
                                    if (materialData.contains("useAlbedoMap")) {
                                        materialPtr->useAlbedoMap = materialData["useAlbedoMap"];
                                    }
                                    if (materialData.contains("useMetallicMap")) {
                                        materialPtr->useMetallicMap = materialData["useMetallicMap"];
                                    }
                                    if (materialData.contains("useRoughnessMap")) {
                                        materialPtr->useRoughnessMap = materialData["useRoughnessMap"];
                                    }
                                    if (materialData.contains("useNormalMap")) {
                                        materialPtr->useNormalMap = materialData["useNormalMap"];
                                    }
                                    if (materialData.contains("useAOMap")) {
                                        materialPtr->useAOMap = materialData["useAOMap"];
                                    }
                                    if (materialData.contains("useDiffuseMap")) {
                                        materialPtr->useDiffuseMap = materialData["useDiffuseMap"];
                                    }
                                    if (materialData.contains("useSpecularMap")) {
                                        materialPtr->useSpecularMap = materialData["useSpecularMap"];
                                    }
                                    
                                    // 加载贴图（如果路径存在且不为空）
                                    if (materialData.contains("albedoMapPath") && 
                                        !materialData["albedoMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["albedoMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("albedoMapFlipY") ? materialData["albedoMapFlipY"].get<bool>() : true;
                                        materialPtr->albedoMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                    if (materialData.contains("metallicMapPath") && 
                                        !materialData["metallicMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["metallicMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("metallicMapFlipY") ? materialData["metallicMapFlipY"].get<bool>() : true;
                                        materialPtr->metallicMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                    if (materialData.contains("roughnessMapPath") && 
                                        !materialData["roughnessMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["roughnessMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("roughnessMapFlipY") ? materialData["roughnessMapFlipY"].get<bool>() : true;
                                        materialPtr->roughnessMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                    if (materialData.contains("normalMapPath") && 
                                        !materialData["normalMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["normalMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("normalMapFlipY") ? materialData["normalMapFlipY"].get<bool>() : true;
                                        materialPtr->normalMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                    if (materialData.contains("aoMapPath") && 
                                        !materialData["aoMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["aoMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("aoMapFlipY") ? materialData["aoMapFlipY"].get<bool>() : true;
                                        materialPtr->aoMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                    if (materialData.contains("diffuseMapPath") && 
                                        !materialData["diffuseMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["diffuseMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("diffuseMapFlipY") ? materialData["diffuseMapFlipY"].get<bool>() : true;
                                        materialPtr->diffuseMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                    if (materialData.contains("specularMapPath") && 
                                        !materialData["specularMapPath"].get<std::string>().empty()) {
                                        std::string texturePath = materialData["specularMapPath"].get<std::string>();
                                        bool flipY = materialData.contains("specularMapFlipY") ? materialData["specularMapFlipY"].get<bool>() : true;
                                        materialPtr->specularMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                                    }
                                }
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "加载模型材质失败: " << e.what() << std::endl;
                        }
                    }
                }
                
                models.push_back(model);
            } catch (const std::exception& e) {
                std::cerr << "加载模型失败: " << e.what() << std::endl;
            }
        }
    }
    
    // 加载点光源
    if (j.contains("pointLights")) {
        for (const auto& l : j["pointLights"]) {
            try {
                auto light = std::make_shared<PointLight>();
                if (l.contains("position")) {
                    auto v = l["position"];
                    light->position = {v[0], v[1], v[2]};
                }
                if (l.contains("diffuse")) {
                    auto c = l["diffuse"];
                    light->diffuse = {c[0], c[1], c[2]};
                }
                if (l.contains("specular")) {
                    auto c = l["specular"];
                    light->specular = {c[0], c[1], c[2]};
                }
                if (l.contains("ambient")) {
                    auto c = l["ambient"];
                    light->ambient = {c[0], c[1], c[2]};
                }
                if (l.contains("intensity")) light->intensity = l["intensity"];
                if (l.contains("constant")) light->constant = l["constant"];
                if (l.contains("linear")) light->linear = l["linear"];
                if (l.contains("quadratic")) light->quadratic = l["quadratic"];
                if (l.contains("shadowEnabled") && l["shadowEnabled"].get<bool>()) {
                    light->SetShadowEnabled(true);
                }
                pointLights.push_back(light);
            } catch (const std::exception& e) {
                std::cerr << "加载点光源失败: " << e.what() << std::endl;
            }
        }
    }
    
    // 加载定向光源
    if (j.contains("directionalLights")) {
        for (const auto& l : j["directionalLights"]) {
            try {
                auto light = std::make_shared<DirectionalLight>();
                if (l.contains("direction")) {
                    auto d = l["direction"];
                    light->direction = {d[0], d[1], d[2]};
                }
                if (l.contains("diffuse")) {
                    auto c = l["diffuse"];
                    light->diffuse = {c[0], c[1], c[2]};
                }
                if (l.contains("specular")) {
                    auto c = l["specular"];
                    light->specular = {c[0], c[1], c[2]};
                }
                if (l.contains("ambient")) {
                    auto c = l["ambient"];
                    light->ambient = {c[0], c[1], c[2]};
                }
                if (l.contains("intensity")) light->intensity = l["intensity"];
                if (l.contains("shadowEnabled") && l["shadowEnabled"].get<bool>()) {
                    light->SetShadowEnabled(true);
                }
                directionalLights.push_back(light);
            } catch (const std::exception& e) {
                std::cerr << "加载定向光源失败: " << e.what() << std::endl;
            }
        }
    }
    
    // 加载聚光灯
    if (j.contains("spotLights")) {
        for (const auto& l : j["spotLights"]) {
            try {
                auto light = std::make_shared<SpotLight>();
                if (l.contains("position")) {
                    auto p = l["position"];
                    light->position = {p[0], p[1], p[2]};
                }
                if (l.contains("direction")) {
                    auto d = l["direction"];
                    light->direction = {d[0], d[1], d[2]};
                }
                if (l.contains("diffuse")) {
                    auto c = l["diffuse"];
                    light->diffuse = {c[0], c[1], c[2]};
                }
                if (l.contains("specular")) {
                    auto c = l["specular"];
                    light->specular = {c[0], c[1], c[2]};
                }
                if (l.contains("ambient")) {
                    auto c = l["ambient"];
                    light->ambient = {c[0], c[1], c[2]};
                }
                if (l.contains("intensity")) light->intensity = l["intensity"];
                if (l.contains("constant")) light->constant = l["constant"];
                if (l.contains("linear")) light->linear = l["linear"];
                if (l.contains("quadratic")) light->quadratic = l["quadratic"];
                if (l.contains("cutOff")) light->cutOff = l["cutOff"];
                if (l.contains("outerCutOff")) light->outerCutOff = l["outerCutOff"];
                if (l.contains("shadowEnabled") && l["shadowEnabled"].get<bool>()) {
                    light->SetShadowEnabled(true);
                }
                spotLights.push_back(light);
            } catch (const std::exception& e) {
                std::cerr << "加载聚光灯失败: " << e.what() << std::endl;
            }
        }
    }
    
    // 加载几何体
    if (j.contains("primitives")) {
        for (const auto& p : j["primitives"]) {
            try {
                Geometry::Type type = static_cast<Geometry::Type>(p["type"].get<int>());
                auto pos = p["position"];
                auto rot = p["rotation"];
                auto scl = p["scale"];
                
                glm::vec3 position(pos[0], pos[1], pos[2]);
                glm::vec3 rotation(rot[0], rot[1], rot[2]);
                glm::vec3 scale(scl[0], scl[1], scl[2]);
                
                // 创建默认材质
                Material defaultMaterial;
                if (p.contains("material")) {
                    const auto& mat = p["material"];
                    if (mat.contains("type"))
                        defaultMaterial.type = static_cast<MaterialType>(mat["type"].get<int>());
                    if (mat.contains("diffuse")) {
                        auto c = mat["diffuse"];
                        defaultMaterial.diffuse = {c[0], c[1], c[2]};
                    }
                    if (mat.contains("specular")) {
                        auto c = mat["specular"];
                        defaultMaterial.specular = {c[0], c[1], c[2]};
                    }
                    if (mat.contains("shininess")) defaultMaterial.shininess = mat["shininess"];
                    if (mat.contains("useDiffuseMap")) defaultMaterial.useDiffuseMap = mat["useDiffuseMap"];
                    if (mat.contains("useSpecularMap")) defaultMaterial.useSpecularMap = mat["useSpecularMap"];
                    if (mat.contains("useNormalMap")) defaultMaterial.useNormalMap = mat["useNormalMap"];
                    if (mat.contains("diffuseMapPath") && !mat["diffuseMapPath"].get<std::string>().empty())
                    {
                        std::string texturePath = mat["diffuseMapPath"].get<std::string>();
                        bool flipY = mat.contains("diffuseMapFlipY") ? mat["diffuseMapFlipY"].get<bool>() : true;
                        std::cout << "正在加载几何体diffuse贴图: " << texturePath << " (flipY=" << flipY << ")" << std::endl;
                        defaultMaterial.diffuseMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                    }
                    if (mat.contains("specularMapPath") && !mat["specularMapPath"].get<std::string>().empty()) {
                        std::string texturePath = mat["specularMapPath"].get<std::string>();
                        bool flipY = mat.contains("specularMapFlipY") ? mat["specularMapFlipY"].get<bool>() : true;
                        std::cout << "正在加载几何体specular贴图: " << texturePath << " (flipY=" << flipY << ")" << std::endl;
                        defaultMaterial.specularMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                    }
                    if (mat.contains("normalMapPath") && !mat["normalMapPath"].get<std::string>().empty()) {
                        std::string texturePath = mat["normalMapPath"].get<std::string>();
                        bool flipY = mat.contains("normalMapFlipY") ? mat["normalMapFlipY"].get<bool>() : true;
                        defaultMaterial.normalMap = TextureManager::GetInstance().GetTexture(texturePath, flipY);
                    }
                }
                
                // 根据参数创建几何体
                if (p.contains("params")) {
                    const auto& params = p["params"];
                    switch (type) {
                        case Geometry::SPHERE: {
                            float radius = params.contains("radius") ? params["radius"].get<float>() : 1.0f;
                            int segments = params.contains("segments") ? params["segments"].get<int>() : 16;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::CUBE: {
                            float width = params.contains("width") ? params["width"].get<float>() : 1.0f;
                            float height = params.contains("height") ? params["height"].get<float>() : 1.0f;
                            float depth = params.contains("depth") ? params["depth"].get<float>() : 1.0f;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::CYLINDER: {
                            float radius = params.contains("radius") ? params["radius"].get<float>() : 1.0f;
                            float height = params.contains("height") ? params["height"].get<float>() : 2.0f;
                            int segments = params.contains("segments") ? params["segments"].get<int>() : 16;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::CONE: {
                            float radius = params.contains("radius") ? params["radius"].get<float>() : 1.0f;
                            float height = params.contains("height") ? params["height"].get<float>() : 2.0f;
                            int segments = params.contains("segments") ? params["segments"].get<int>() : 16;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::PRISM: {
                            int sides = params.contains("sides") ? params["sides"].get<int>() : 3;
                            float radius = params.contains("radius") ? params["radius"].get<float>() : 1.0f;
                            float height = params.contains("height") ? params["height"].get<float>() : 2.0f;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::PYRAMID: {
                            int sides = params.contains("sides") ? params["sides"].get<int>() : 4;
                            float radius = params.contains("radius") ? params["radius"].get<float>() : 1.0f;
                            float height = params.contains("height") ? params["height"].get<float>() : 2.0f;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::TORUS: {
                            float majorRadius = params.contains("majorRadius") ? params["majorRadius"].get<float>() : 1.0f;
                            float minorRadius = params.contains("minorRadius") ? params["minorRadius"].get<float>() : 0.3f;
                            int majorSegments = params.contains("majorSegments") ? params["majorSegments"].get<int>() : 16;
                            int minorSegments = params.contains("minorSegments") ? params["minorSegments"].get<int>() : 12;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::ELLIPSOID: {
                            float radiusX = params.contains("radiusX") ? params["radiusX"].get<float>() : 1.0f;
                            float radiusY = params.contains("radiusY") ? params["radiusY"].get<float>() : 1.0f;
                            float radiusZ = params.contains("radiusZ") ? params["radiusZ"].get<float>() : 1.0f;
                            int segments = params.contains("segments") ? params["segments"].get<int>() : 16;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::FRUSTUM: {
                            float radiusTop = params.contains("radiusTop") ? params["radiusTop"].get<float>() : 0.5f;
                            float radiusBottom = params.contains("radiusBottom") ? params["radiusBottom"].get<float>() : 1.0f;
                            float height = params.contains("height") ? params["height"].get<float>() : 2.0f;
                            int segments = params.contains("segments") ? params["segments"].get<int>() : 16;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        case Geometry::ARROW: {
                            float length = params.contains("length") ? params["length"].get<float>() : 1.0f;
                            float headSize = params.contains("headSize") ? params["headSize"].get<float>() : 0.2f;
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);

                            break;
                        }
                        // 其他几何体类型类似处理...
                        default:
                            CreatePrimitive(type, position, scale, rotation, defaultMaterial);
                            break;
                    }
                } else {
                    CreatePrimitive(type, position, scale, rotation, defaultMaterial);
                }
            } catch (const std::exception& e) {
                std::cerr << "加载几何体失败: " << e.what() << std::endl;
            }
        }
    }
    
    // 加载相机状态
    if (j.contains("camera") && mainCamera) {
        try {
            const auto& cam = j["camera"];
            if (cam.contains("position")) {
                auto p = cam["position"];
                mainCamera->Position = glm::vec3(p[0], p[1], p[2]);
            }
            if (cam.contains("front")) {
                auto f = cam["front"];
                mainCamera->Front = glm::vec3(f[0], f[1], f[2]);
            }
            if (cam.contains("up")) {
                auto u = cam["up"];
                mainCamera->Up = glm::vec3(u[0], u[1], u[2]);
            }
            if (cam.contains("yaw")) mainCamera->Yaw = cam["yaw"];
            if (cam.contains("pitch")) mainCamera->Pitch = cam["pitch"];
            if (cam.contains("fov")) mainCamera->Zoom = cam["fov"];
            if (cam.contains("movementSpeed")) mainCamera->MovementSpeed = cam["movementSpeed"];
            if (cam.contains("mouseSensitivity")) mainCamera->MouseSensitivity = cam["mouseSensitivity"];
        } catch (const std::exception& e) {
            std::cerr << "加载相机状态失败: " << e.what() << std::endl;
        }
    }
    
    // 加载渲染设置
    if (j.contains("renderSettings")) {
        try {
            const auto& settings = j["renderSettings"];
            if (settings.contains("renderMode")) {
                SetRenderMode(static_cast<RenderMode>(settings["renderMode"].get<int>()));
            }
            if (settings.contains("shadowEnabled")) SetShadow(settings["shadowEnabled"]);
            if (settings.contains("hdrEnabled")) SetHDR(settings["hdrEnabled"]);
            if (settings.contains("bloomEnabled")) SetBloom(settings["bloomEnabled"]);
            if (settings.contains("ssaoEnabled")) SetSSAO(settings["ssaoEnabled"]);
            if (settings.contains("msaaEnabled")) {
                SetMSAA(settings["msaaEnabled"], 4); // 默认4倍采样
            }
            if (settings.contains("fxaaEnabled")) fxaaEnabled = settings["fxaaEnabled"];
            if (settings.contains("gammaCorrection")) SetGammaCorrection(settings["gammaCorrection"]);
            if (settings.contains("iblEnabled")) SetIBL(settings["iblEnabled"]);
            if (settings.contains("showLights")) showLights = settings["showLights"];
            if (settings.contains("backgroundType")) {
                SetBackgroundType(static_cast<BackgroundType>(settings["backgroundType"].get<int>()));
            }
        } catch (const std::exception& e) {
            std::cerr << "加载渲染设置失败: " << e.what() << std::endl;
        }
    }

    std::cout << "场景已加载成功: " << path << std::endl;
}

Renderer::Renderer(int width, int height) : width(width), height(height)
{
}

Renderer::~Renderer()
{
    // 清理资源
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteTextures(1, &ssaoNoiseTexture);
    
    // 清理IBL资源
    glDeleteTextures(1, &envCubemap);
    glDeleteTextures(1, &irradianceMap);
    glDeleteTextures(1, &prefilterMap);
    glDeleteTextures(1, &brdfLUTTexture);
    
    // 清理IBL帧缓冲区（Framebuffer对象会自动清理）
    iblCaptureBuffer.reset();
    
    // 清理多光源阴影缓冲区
    ClearLightShadowBuffers();
    
    for (auto &model : models)
    {
        model.reset();
    }
    models.clear();
    for (auto &shader : shaders)
    {
        shader.second.reset();
    }
    shaders.clear();
    for (auto &pointLight : pointLights)
    {
        pointLight.reset();
    }
    pointLights.clear();
    for (auto &directionalLight : directionalLights)
    {
        directionalLight.reset();
    }
    directionalLights.clear();
    for (auto &spotLight : spotLights)
    {
        spotLight.reset();
    }
    spotLights.clear();
    for (auto &primitive : primitives)
    {
        primitive.mesh->SetMaterial(nullptr);
        primitive.mesh.reset();
    }
    primitives.clear();
    gBuffer.reset();
    shadowBuffer.reset();
    hdrBuffer.reset();
    hdrBufferMS.reset();
    bloomPrefilterBuffer.reset();
    for (auto &blurBuffer : bloomBlurBuffers)
    {
        blurBuffer.reset();
    }
    ssaoBuffer.reset();
    forwardShader.reset();
    pbrShader.reset();
    deferredGeometryShader.reset();
    deferredLightingShader.reset();
    pbrDeferredGeometryShader.reset();
    shadowDepthShader.reset();
    skyboxShader.reset();
    hdrShader.reset();
    postProcessShader.reset();
    postShaderMS.reset();
    bloomPreShader.reset();
    bloomBlurShader.reset();
    ssaoShader.reset();
    equirectangularToCubemapShader.reset();
    irradianceShader.reset();
    prefilterShader.reset();
    brdfShader.reset();
    environmentMap.reset();
    mainCamera.reset();
}

void Renderer::BeginFrame()
{
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame()
{
}

void Renderer::Update(float deltaTime)
{
    // 更新相机
    mainCamera->Update(deltaTime);

    // // 更新光源
    // for (auto &light : lights)
    // {
    //     light->Update(deltaTime);
    // }

    // // 更新模型
    // for (auto &model : models)
    // {
    //     model->Update(deltaTime);
    // }

    // // 更新几何体
    // for (auto &primitive : primitives)
    // {
    //     primitive.Update(deltaTime);
    // }
}

void Renderer::Initialize()
{
    // 加载着色器
    forwardShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/forward/blinn_phong.vert"),
                                             FileSystem::GetPath("resources/shaders/forward/blinn_phong.frag"));

    pbrShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/forward/pbr.vert"),
                                         FileSystem::GetPath("resources/shaders/forward/pbr.frag"));

    deferredGeometryShader =
        std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/deferred/geometry_pass.vert"),
                                 FileSystem::GetPath("resources/shaders/deferred/geometry_pass.frag"));

    deferredLightingShader =
        std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/deferred/lighting_pass.vert"),
                                 FileSystem::GetPath("resources/shaders/deferred/lighting_pass.frag"));

    pbrDeferredGeometryShader =
        std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/deferred/pbr_geometry_pass.vert"),
                                 FileSystem::GetPath("resources/shaders/deferred/pbr_geometry_pass.frag"));

    lightsShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/utility/light.vert"),
                                            FileSystem::GetPath("resources/shaders/utility/light.frag"));

    shadowDepthShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/utility/shadow_depth.vert"),
                                                 FileSystem::GetPath("resources/shaders/utility/shadow_depth.frag"));

    postProcessShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
                                                 FileSystem::GetPath("resources/shaders/postprocess/post.frag"));
    postShaderMS = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
                                            FileSystem::GetPath("resources/shaders/postprocess/post_msaa.frag"));

    ssaoShader =
        std::make_unique<Shader>("resources/shaders/postprocess/quad.vert", "resources/shaders/postprocess/ssao.frag");
    ssaoBlurShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
                                              FileSystem::GetPath("resources/shaders/postprocess/ssao_blur.frag"));
    
    bloomPreShader = std::make_unique<Shader>("resources/shaders/postprocess/quad.vert",
                                              "resources/shaders/postprocess/bloom_prefilter.frag");
    bloomBlurShader = std::make_unique<Shader>("resources/shaders/postprocess/quad.vert",
                                               "resources/shaders/postprocess/bloom_blur.frag");
    fxaaShader = std::make_unique<Shader>(
        FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
        FileSystem::GetPath("resources/shaders/postprocess/fxaa.frag"));
    
    // IBL着色器
    equirectangularToCubemapShader = std::make_unique<Shader>(
        FileSystem::GetPath("resources/shaders/ibl/cubemap.vert"),
        FileSystem::GetPath("resources/shaders/ibl/equirectangular_to_cubemap.frag"));
    
    irradianceShader = std::make_unique<Shader>(
        FileSystem::GetPath("resources/shaders/ibl/cubemap.vert"),
        FileSystem::GetPath("resources/shaders/ibl/irradiance_convolution.frag"));
    
    prefilterShader = std::make_unique<Shader>(
        FileSystem::GetPath("resources/shaders/ibl/cubemap.vert"),
        FileSystem::GetPath("resources/shaders/ibl/prefilter.frag"));
    
    brdfShader = std::make_unique<Shader>(
        FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
        FileSystem::GetPath("resources/shaders/ibl/brdf_lut.frag"));
    
    // 初始化帧缓冲
    SetupShadowBuffer();
    SetupGBuffer();
    SetupHDRBuffer();
    SetupHDRBufferMS();
    SetupBloomBuffer();
    SetupSSAOBuffer();
    SetupFXAABuffer();
    SetupViewportBuffer();

    GenerateSSAOKernel();
    GenerateSSAONoiseTexture();

    // 设置IBL
    SetupIBL();

    environmentMap = std::make_shared<Texture>();
    environmentMap->LoadCubemap({FileSystem::GetPath("resources/textures/skybox/right.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/left.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/top.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/bottom.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/front.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/back.jpg")});

    // 设置天空盒
    SetupSkybox();

    // 设置相机
    mainCamera = std::make_shared<Camera>();
}

void Renderer::SetupGBuffer()
{
    gBuffer = std::make_unique<Framebuffer>(width, height);

    // 位置、法线、反照率、金属度/粗糙度/ao、漫反射/镜面反射贴图
    gBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);      // 位置 + 深度
    gBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);      // 法线
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 漫反射贴图
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 镜面反射贴图
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 金属度
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 粗糙度
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // AO
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 环境光贴图

    gBuffer->AddDepthBuffer();
    gBuffer->CheckComplete();
}

void Renderer::SetupShadowBuffer()
{
    shadowBuffer = std::make_unique<Framebuffer>(2048, 2048);
    shadowBuffer->AddDepthTexture();
    shadowBuffer->CheckComplete();
}

void Renderer::SetupHDRBuffer()
{
    hdrBuffer = std::make_unique<Framebuffer>(width, height);
    hdrBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    hdrBuffer->AddDepthBuffer();
    hdrBuffer->CheckComplete();
}
void Renderer::SetupHDRBufferMS()
{
    hdrBufferMS = std::make_unique<Framebuffer>(width, height);
    hdrBufferMS->AddColorTextureMultisample(GL_RGBA16F, msaaSamples); // 使用动态采样数
    hdrBufferMS->AddDepthBufferMultisample(msaaSamples);
    hdrBufferMS->CheckComplete();
}

void Renderer::SetupBloomBuffer()
{
    bloomPrefilterBuffer = std::make_unique<Framebuffer>(width, height);
    bloomPrefilterBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    bloomPrefilterBuffer->CheckComplete();

    for (int i = 0; i < 2; ++i)
    {
        bloomBlurBuffers[i] = std::make_unique<Framebuffer>(width / (1 << i), height / (1 << i));
        bloomBlurBuffers[i]->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
        bloomBlurBuffers[i]->CheckComplete();
    }
}

void Renderer::SetupSSAOBuffer()
{
    // 主SSAO缓冲
    ssaoBuffer = std::make_unique<Framebuffer>(width, height);
    ssaoBuffer->AddColorTexture(GL_RED, GL_RED, GL_FLOAT); // 单通道浮点纹理
    ssaoBuffer->CheckComplete();

    // SSAO模糊缓冲
    ssaoBlurBuffer = std::make_unique<Framebuffer>(width, height);
    ssaoBlurBuffer->AddColorTexture(GL_RED, GL_RED, GL_FLOAT);
    ssaoBlurBuffer->CheckComplete();
}

void Renderer::SetupFXAABuffer()
{
    fxaaBuffer = std::make_unique<Framebuffer>(width, height);
    fxaaBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    fxaaBuffer->CheckComplete();
}

void Renderer::SetupViewportBuffer()
{
    viewportBuffer = std::make_unique<Framebuffer>(width, height);
    viewportBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    viewportBuffer->CheckComplete();
}

void Renderer::SetupSkybox()
{
    float skyboxVertices[] = {// positions
                              -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    skyboxShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/utility/skybox.vert"),
                                            FileSystem::GetPath("resources/shaders/utility/skybox.frag"));
}

void Renderer::RenderScene()
{
    if (shadowEnabled)
    {
        RenderShadows();
    }

    switch (currentMode)
    {
    case FORWARD:
        RenderForward();
        break;
    case DEFERRED:
        RenderDeferred();
        break;
    }

    if (showLights)
    {
        // 渲染光源
        RenderLights();
    }

    RenderPostProcessing();
}

void Renderer::RenderForward()
{
    if (msaaEnabled)
    {
        hdrBufferMS->Bind();
    }
    else
    {
        hdrBuffer->Bind();
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // 分别处理Blinn-Phong材质和PBR材质的物体
    
    // 1. 渲染Blinn-Phong材质的物体
    forwardShader->Use();
    // 设置相机、光照等uniform
    glm::mat4 view = mainCamera->GetViewMatrix();
    glm::mat4 projection = mainCamera->GetProjectionMatrix(static_cast<float>(width) / height);
    forwardShader->SetMat4("view", view);
    forwardShader->SetMat4("projection", projection);
    forwardShader->SetVec3("viewPos", mainCamera->Position);

    // 设置光源
    forwardShader->SetInt("numLights[0]", directionalLights.size()); // 方向光数量
    forwardShader->SetInt("numLights[1]", pointLights.size());       // 点光
    forwardShader->SetInt("numLights[2]", spotLights.size());        // 聚光灯数量
    
    // 启用阴影
    if (shadowEnabled)
    {
        forwardShader->SetBool("shadowEnabled", true);
    }
    else
    {
        forwardShader->SetBool("shadowEnabled", false);
    }
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        pointLights[i]->SetupShader(*forwardShader, i, shadowEnabled);
    }
    for (size_t i = 0; i < directionalLights.size(); ++i)
    {
        directionalLights[i]->SetupShader(*forwardShader, i, shadowEnabled);
    }
    for (size_t i = 0; i < spotLights.size(); ++i)
    {
        spotLights[i]->SetupShader(*forwardShader, i, shadowEnabled);
    }
    
    // 渲染Blinn-Phong材质的模型
    for (auto &model : models)
    {
        model->DrawWithMaterialType(*forwardShader, BLINN_PHONG);
    }

    // 渲染Blinn-Phong材质的几何体
    for (auto &primitive : primitives)
    {
        if (primitive.mesh->GetMaterial()->type == BLINN_PHONG)
        {
            primitive.mesh->Draw(*forwardShader);
        }
    }

    // 2. 渲染PBR材质的物体
    pbrShader->Use();
    
    // 设置相机矩阵
    pbrShader->SetMat4("view", view);
    pbrShader->SetMat4("projection", projection);
    pbrShader->SetVec3("viewPos", mainCamera->Position);

    // 设置光源数量
    pbrShader->SetInt("numLights[0]", directionalLights.size());
    pbrShader->SetInt("numLights[1]", pointLights.size());
    pbrShader->SetInt("numLights[2]", spotLights.size());
    
    // 设置阴影
    pbrShader->SetBool("shadowEnabled", shadowEnabled);
    
    // 设置光源参数
    for (size_t i = 0; i < directionalLights.size(); ++i)
    {
        auto& light = directionalLights[i];
        std::string base = "dirLights[" + std::to_string(i) + "]";
        pbrShader->SetVec3(base + ".direction", light->direction);
        pbrShader->SetVec3(base + ".color", light->diffuse);
        pbrShader->SetFloat(base + ".intensity", light->intensity);
        pbrShader->SetBool(base + ".shadowEnabled", light->HasShadows() && shadowEnabled);
        if (light->HasShadows() && shadowEnabled)
        {
            pbrShader->SetMat4(base + ".lightSpaceMatrix", light->GetLightSpaceMatrix());
            // 绑定阴影贴图
            glActiveTexture(GL_TEXTURE10 + i);
            glBindTexture(GL_TEXTURE_2D, light->GetShadowMap());
            pbrShader->SetInt(base + ".shadowMap", 10 + i);
        }
    }
    
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        auto& light = pointLights[i];
        std::string base = "pointLights[" + std::to_string(i) + "]";
        pbrShader->SetVec3(base + ".position", light->position);
        pbrShader->SetVec3(base + ".color", light->diffuse);
        pbrShader->SetFloat(base + ".intensity", light->intensity);
        pbrShader->SetFloat(base + ".constant", light->constant);
        pbrShader->SetFloat(base + ".linear", light->linear);
        pbrShader->SetFloat(base + ".quadratic", light->quadratic);
        pbrShader->SetBool(base + ".shadowEnabled", light->HasShadows() && shadowEnabled);
    }
    
    for (size_t i = 0; i < spotLights.size(); ++i)
    {
        auto& light = spotLights[i];
        std::string base = "spotLights[" + std::to_string(i) + "]";
        pbrShader->SetVec3(base + ".position", light->position);
        pbrShader->SetVec3(base + ".direction", light->direction);
        pbrShader->SetVec3(base + ".color", light->diffuse);
        pbrShader->SetFloat(base + ".intensity", light->intensity);
        pbrShader->SetFloat(base + ".constant", light->constant);
        pbrShader->SetFloat(base + ".linear", light->linear);
        pbrShader->SetFloat(base + ".quadratic", light->quadratic);
        pbrShader->SetFloat(base + ".cutOff", light->cutOff);
        pbrShader->SetFloat(base + ".outerCutOff", light->outerCutOff);
        pbrShader->SetBool(base + ".shadowEnabled", light->HasShadows() && shadowEnabled);
        if (light->HasShadows() && shadowEnabled)
        {
            pbrShader->SetMat4(base + ".lightSpaceMatrix", light->GetLightSpaceMatrix());
        }
    }

    // 设置IBL
    pbrShader->SetBool("iblEnabled", iblEnabled);
    if (iblEnabled)
    {
        glActiveTexture(GL_TEXTURE20);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        pbrShader->SetInt("irradianceMap", 20);

        glActiveTexture(GL_TEXTURE21);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        pbrShader->SetInt("prefilterMap", 21);

        glActiveTexture(GL_TEXTURE22);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        pbrShader->SetInt("brdfLUT", 22);
    }

    // 渲染PBR材质的模型
    for (auto &model : models)
    {
        model->DrawWithMaterialType(*pbrShader, PBR);
    }

    // 渲染PBR材质的几何体
    for (auto &primitive : primitives)
    {
        if (primitive.mesh->GetMaterial()->type == PBR)
        {
            primitive.mesh->Draw(*pbrShader);
        }
    }

    if (iblEnabled)
    {
        RenderSkybox();
    }
}

void Renderer::RenderDeferred()
{
    // 几何处理阶段
    gBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto view = mainCamera->GetViewMatrix();
    auto projection = mainCamera->GetProjectionMatrix(static_cast<float>(width) / height);

    // 1. 渲染Blinn-Phong材质的物体
    deferredGeometryShader->Use();
    deferredGeometryShader->SetMat4("view", view);
    deferredGeometryShader->SetMat4("projection", projection);
    deferredGeometryShader->SetVec3("viewPos", mainCamera->Position);

    // 渲染Blinn-Phong材质的模型
    for (auto &model : models)
    {
        model->DrawWithMaterialType(*deferredGeometryShader, BLINN_PHONG);
    }

    // 渲染Blinn-Phong材质的几何体
    for (auto &primitive : primitives)
    {
        if (primitive.mesh->GetMaterial()->type == BLINN_PHONG)
        {
            primitive.mesh->Draw(*deferredGeometryShader);
        }
    }

    // 2. 渲染PBR材质的物体
    pbrDeferredGeometryShader->Use();
    pbrDeferredGeometryShader->SetMat4("view", view);
    pbrDeferredGeometryShader->SetMat4("projection", projection);
    pbrDeferredGeometryShader->SetVec3("viewPos", mainCamera->Position);

    // 渲染PBR材质的模型
    for (auto &model : models)
    {
        model->DrawWithMaterialType(*pbrDeferredGeometryShader, PBR);
    }

    // 渲染PBR材质的几何体
    for (auto &primitive : primitives)
    {
        if (primitive.mesh->GetMaterial()->type == PBR)
        {
            primitive.mesh->Draw(*pbrDeferredGeometryShader);
        }
    }

    if (ssaoEnabled)
    {
        RenderSSAO();
    }

    // 光照处理阶段
    GLuint targetFBO = msaaEnabled ? hdrBufferMS->GetID() : hdrBuffer->GetID();
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glDepthMask(GL_FALSE);     // 不写入深度
    glEnable(GL_STENCIL_TEST); // 全程开模板
    glDisable(GL_CULL_FACE);   // 由每个 pass 自己决定

    deferredLightingShader->Use();
    // 绑定GBuffer纹理并设置uniform

    // 设置相机
    deferredLightingShader->SetMat4("view", mainCamera->GetViewMatrix());
    deferredLightingShader->SetMat4("projection", mainCamera->GetProjectionMatrix(static_cast<float>(width) / height));
    deferredLightingShader->SetVec3("viewPos", mainCamera->Position);
    deferredLightingShader->SetVec2("screenSize", glm::vec2(width, height));
    
    // 设置全局阴影开关
    deferredLightingShader->SetBool("shadowEnabled", shadowEnabled);

    // 设置IBL参数
    deferredLightingShader->SetBool("iblEnabled", iblEnabled);
    if (iblEnabled)
    {
        glActiveTexture(GL_TEXTURE20);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        deferredLightingShader->SetInt("irradianceMap", 20);

        glActiveTexture(GL_TEXTURE21);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        deferredLightingShader->SetInt("prefilterMap", 21);

        glActiveTexture(GL_TEXTURE22);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        deferredLightingShader->SetInt("brdfLUT", 22);
    }

    const int texSlots[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    const char *texNames[8] = {"gPosition", "gNormal", "gAlbedo", "gSpecular", "gMetallic", "gRoughness", "gAo", "gAmbient"};
    for (int i = 0; i < 8; ++i)
    {
        deferredLightingShader->SetInt(texNames[i], texSlots[i]);
        gBuffer->BindTexture(i, texSlots[i]);
    }
    deferredLightingShader->SetBool("ssaoEnabled", ssaoEnabled);
    if (ssaoEnabled)
    {
        ssaoBlurBuffer->BindTexture(0, 8); // 将模糊后的SSAO纹理绑定到槽8
        deferredLightingShader->SetInt("ssao", 8);
    }

    for (const auto &light : GetLights())
    {
        if (light->getType() != 1)
        {
            // 调试输出：记录光源类型
            const char* lightTypeName = "Unknown";
            switch(light->getType()) {
                case 0: lightTypeName = "PointLight"; break;
                case 1: lightTypeName = "DirectionalLight"; break;
                case 2: lightTypeName = "SpotLight"; break;
            }
            // std::cout << "延迟渲染处理 " << lightTypeName << " (type=" << light->getType() << ")" << std::endl;
            
            // === 第一步：标记光体积区域 ===
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 不写颜色
            glDepthMask(GL_FALSE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE); // 正背面都要

            glStencilFunc(GL_ALWAYS, 0, 0);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

            // 绘制光体积（仅更新模板缓冲区）
            light->drawLightMesh(deferredLightingShader);
            // 注意：此时光体积的模板值会被设置为1（或其他非0值），表示该区域有光照影响

            // === 第二步：渲染光照（仅标记区域） ===
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);

            // 再次绘制光体积（实际渲染光照）
            light->drawLightMesh(deferredLightingShader);
            glCullFace(GL_BACK);
            glDisable(GL_BLEND);
        }
        else
        {
            glDisable(GL_STENCIL_TEST); // 关闭模板测试
            glDisable(GL_CULL_FACE);    // 关闭面剔除
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
            light->drawLightMesh(deferredLightingShader);
        }
    }

    // 恢复状态
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    if (iblEnabled)
    {
        RenderSkybox();
    }
}

void Renderer::RenderShadows()
{
    if (!shadowEnabled) return;

    // 保存当前OpenGL状态
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLint prevFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);

    shadowDepthShader->Use();

    // 渲染定向光阴影
    for (size_t i = 0; i < directionalLights.size(); ++i)
    {
        auto &dirLight = directionalLights[i];
        if (dirLight->HasShadows())
        {
            // 为每个光源创建独立的阴影缓冲区
            std::unique_ptr<Framebuffer> lightShadowBuffer = std::make_unique<Framebuffer>(2048, 2048);
            lightShadowBuffer->AddDepthTexture();
            lightShadowBuffer->CheckComplete();
            
            lightShadowBuffer->Bind();
            glViewport(0, 0, lightShadowBuffer->GetWidth(), lightShadowBuffer->GetHeight());
            glClear(GL_DEPTH_BUFFER_BIT);

            // 设置光源空间矩阵
            glm::mat4 lightSpaceMatrix = dirLight->GetLightSpaceMatrix();
            shadowDepthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

            // 渲染所有模型和几何体
            for (auto &model : models)
            {
                // 构建模型矩阵
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, model->GetPosition());
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
                modelMatrix = glm::scale(modelMatrix, model->GetScale());
                
                shadowDepthShader->SetMat4("model", modelMatrix);
                model->Draw(*shadowDepthShader);
            }

            for (auto &primitive : primitives)
            {
                // 构建几何体矩阵
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, primitive.position);
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                modelMatrix = glm::scale(modelMatrix, primitive.scale);
                
                shadowDepthShader->SetMat4("model", modelMatrix);
                primitive.mesh->Draw(*shadowDepthShader);
            }

            // 将阴影贴图绑定到光源
            unsigned int shadowMap = lightShadowBuffer->GetDepthTexture();
            dirLight->SetShadowMap(shadowMap);
            
            // 将缓冲区保存到管理容器中（临时方案）
            if (lightShadowBuffers.size() <= i) {
                lightShadowBuffers.resize(i + 1);
            }
            lightShadowBuffers[i] = std::move(lightShadowBuffer);
        }
    }

    // 渲染点光源阴影
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        auto &pointLight = pointLights[i];
        if (pointLight->HasShadows())
        {
            // 为每个点光源创建独立的阴影缓冲区
            std::unique_ptr<Framebuffer> lightShadowBuffer = std::make_unique<Framebuffer>(1024, 1024);
            lightShadowBuffer->AddDepthTexture();
            lightShadowBuffer->CheckComplete();

            lightShadowBuffer->Bind();
            glViewport(0, 0, lightShadowBuffer->GetWidth(), lightShadowBuffer->GetHeight());
            glClear(GL_DEPTH_BUFFER_BIT);

            // 设置光源空间矩阵（使用透视投影）
            glm::mat4 lightSpaceMatrix = pointLight->GetLightSpaceMatrix();
            shadowDepthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

            // 渲染所有模型和几何体
            for (auto &model : models)
            {
                // 构建模型矩阵
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, model->GetPosition());
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
                modelMatrix = glm::scale(modelMatrix, model->GetScale());
                
                shadowDepthShader->SetMat4("model", modelMatrix);
                model->Draw(*shadowDepthShader);
            }

            for (auto &primitive : primitives)
            {
                // 构建几何体矩阵
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, primitive.position);
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                modelMatrix = glm::scale(modelMatrix, primitive.scale);
                
                shadowDepthShader->SetMat4("model", modelMatrix);
                primitive.mesh->Draw(*shadowDepthShader);
            }

            // 将阴影贴图绑定到光源
            unsigned int shadowMap = lightShadowBuffer->GetDepthTexture();
            pointLight->SetShadowMap(shadowMap);
            
            // 保存缓冲区，使用偏移索引以避免与定向光冲突
            size_t bufferIndex = directionalLights.size() + i;
            if (lightShadowBuffers.size() <= bufferIndex) {
                lightShadowBuffers.resize(bufferIndex + 1);
            }
            lightShadowBuffers[bufferIndex] = std::move(lightShadowBuffer);
        }
    }

    // 渲染聚光灯阴影
    for (size_t i = 0; i < spotLights.size(); ++i)
    {
        auto &spotLight = spotLights[i];
        if (spotLight->HasShadows())
        {
            // 为每个聚光灯创建独立的阴影缓冲区
            std::unique_ptr<Framebuffer> lightShadowBuffer = std::make_unique<Framebuffer>(1024, 1024);
            lightShadowBuffer->AddDepthTexture();
            lightShadowBuffer->CheckComplete();

            lightShadowBuffer->Bind();
            glViewport(0, 0, lightShadowBuffer->GetWidth(), lightShadowBuffer->GetHeight());
            glClear(GL_DEPTH_BUFFER_BIT);

            // 设置光源空间矩阵
            glm::mat4 lightSpaceMatrix = spotLight->GetLightSpaceMatrix();
            shadowDepthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

            // 渲染所有模型和几何体
            for (auto &model : models)
            {
                // 构建模型矩阵
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, model->GetPosition());
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(model->GetRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
                modelMatrix = glm::scale(modelMatrix, model->GetScale());
                
                shadowDepthShader->SetMat4("model", modelMatrix);
                model->Draw(*shadowDepthShader);
            }

            for (auto &primitive : primitives)
            {
                // 构建几何体矩阵
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, primitive.position);
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(primitive.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                modelMatrix = glm::scale(modelMatrix, primitive.scale);
                
                shadowDepthShader->SetMat4("model", modelMatrix);
                primitive.mesh->Draw(*shadowDepthShader);
            }

            // 将阴影贴图绑定到光源
            unsigned int shadowMap = lightShadowBuffer->GetDepthTexture();
            spotLight->SetShadowMap(shadowMap);
            
            // 保存缓冲区，使用偏移索引
            size_t bufferIndex = directionalLights.size() + pointLights.size() + i;
            if (lightShadowBuffers.size() <= bufferIndex) {
                lightShadowBuffers.resize(bufferIndex + 1);
            }
            lightShadowBuffers[bufferIndex] = std::move(lightShadowBuffer);
        }
    }

    // 恢复OpenGL状态
    glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void Renderer::RenderSkybox()
{
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    
    skyboxShader->Use();
    
    // 设置视图矩阵(移除平移部分)
    glm::mat4 view = glm::mat4(glm::mat3(mainCamera->GetViewMatrix()));
    skyboxShader->SetMat4("view", view);
    skyboxShader->SetMat4("projection", mainCamera->GetProjectionMatrix(static_cast<float>(width) / height));
    
    if (gammaCorrection)
    {
        skyboxShader->SetBool("gammaEnabled", true);
    }
    else
    {
        skyboxShader->SetBool("gammaEnabled", false);
    }

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    
    // 根据背景类型选择不同的纹理
    if (backgroundType == HDR_ENVIRONMENT && iblEnabled && envCubemap != 0)
    {
        // 使用HDR环境贴图
        skyboxShader->SetInt("skybox", 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    }
    else if (environmentMap)
    {
        // 使用传统skybox纹理
        skyboxShader->SetInt("skybox", 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap->GetID());
    }
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void Renderer::RenderSSAO()
{
    // 第一步：生成SSAO纹理
    ssaoBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoShader->Use();

    // 绑定GBuffer纹理
    gBuffer->BindTexture(0, 0); // 位置
    gBuffer->BindTexture(1, 1); // 法线
    ssaoShader->SetInt("gPosition", 0);
    ssaoShader->SetInt("gNormal", 1);

    // 绑定噪声纹理
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
    ssaoShader->SetInt("texNoise", 2);

    // 传递采样核心
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        ssaoShader->SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    }

    // 设置矩阵
    glm::mat4 projection = mainCamera->GetProjectionMatrix(static_cast<float>(width) / height);
    ssaoShader->SetMat4("projection", projection);
    ssaoShader->SetMat4("view", mainCamera->GetViewMatrix());

    // 设置参数
    ssaoShader->SetVec2("noiseScale", glm::vec2(width / (float)ssaoNoiseSize, height / (float)ssaoNoiseSize));
    ssaoShader->SetInt("kernelSize", ssaoKernelSize);
    ssaoShader->SetFloat("radius", 0.5f);
    ssaoShader->SetFloat("bias", 0.025f);
    ssaoShader->SetFloat("power", 1.0f);

    // 渲染全屏四边形
    RenderQuad();

    // 第二步：模糊SSAO纹理
    ssaoBlurBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoBlurShader->Use();
    ssaoBlurShader->SetInt("ssaoInput", 0);
    ssaoBuffer->BindTexture(0, 0);
    RenderQuad();

    // 解绑帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderLights()
{
    if (msaaEnabled)
    {
        hdrBufferMS->Bind();
    }
    else
    {
        hdrBuffer->Bind();
    }
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    lightsShader->Use();
    lightsShader->SetMat4("view", mainCamera->GetViewMatrix());
    lightsShader->SetMat4("projection", mainCamera->GetProjectionMatrix(width * 1.0f / height));

    for (const std::shared_ptr<Light> &light : GetLights())
    {
        std::shared_ptr<Mesh> lightMesh;
        glm::vec3 scale(0.2f);
        glm::vec3 rotation(0.0f);

        switch (light->getType())
        {
        case 0:
            lightMesh = Geometry::CreateSphere();
            lightMesh->SetTransform(light->getPosition(), rotation, scale);
            break;
        case 2: {
            auto spotLight = std::dynamic_pointer_cast<SpotLight>(light);
            if (spotLight)
            {
                // 为可视化目的使用更合理的光锥尺寸
                // 不使用完整的理论衰减距离，而是使用一个合适的显示范围
                
                // 方案1：使用固定的合理显示距离
                float visualRange = 5.0f; // 合适的可视化距离
                
                // 方案2：基于光源强度的自适应范围（更智能）
                float intensityFactor = glm::clamp(spotLight->intensity, 0.1f, 2.0f);
                visualRange = 2.0f + intensityFactor * 3.0f; // 2-8单位的范围
                
                // outerCutOff已经是余弦值，还原为弧度
                float outerAngleRad = glm::acos(spotLight->outerCutOff);
                float coneRadius = visualRange * glm::tan(outerAngleRad);
                
                // 限制最大半径，避免过大的可视化
                coneRadius = glm::min(coneRadius, visualRange * 0.8f);

                // 创建圆锥（顶部半径为0，底部半径为计算值）
                lightMesh = Geometry::CreateFrustum(0.0f, coneRadius, visualRange, 16);

                // 使用四元数计算正确的旋转
                glm::vec3 defaultDir = glm::vec3(0.0f, 1.0f, 0.0f); // 圆锥默认朝向Y轴正方向
                glm::vec3 targetDir = glm::normalize(spotLight->direction);
                
                glm::quat rotationQuat;
                float dot = glm::dot(defaultDir, targetDir);
                if (dot < -0.999999f) {
                    // 180度旋转的特殊情况
                    rotationQuat = glm::angleAxis(glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
                } else if (dot > 0.999999f) {
                    // 方向相同，无需旋转
                    rotationQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                } else {
                    // 一般情况下的旋转
                    glm::vec3 axis = glm::normalize(glm::cross(defaultDir, targetDir));
                    float angle = glm::acos(glm::clamp(dot, -1.0f, 1.0f));
                    rotationQuat = glm::angleAxis(angle, axis);
                }
                
                // 转换四元数为欧拉角用于SetTransform
                glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(rotationQuat));
                
                // 调整位置，使圆锥顶点在光源位置
                // 由于圆锥几何顶点在+Y，我们需要调整位置
                glm::vec3 offset = -targetDir * (visualRange * 0.5f);
                glm::vec3 adjustedPosition = spotLight->position + offset;
                
                // 设置变换
                lightMesh->SetTransform(adjustedPosition, eulerAngles, glm::vec3(1.0f));
            }
            break;
        }
        }

        if (lightMesh)
        {
            lightsShader->SetVec3("lightColor", light->getLightColor());
            lightMesh->Draw(*lightsShader);
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void Renderer::RenderQuad()
{
    static GLuint quadVAO = 0, quadVBO = 0;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::RenderCube()
{
    if (cubeVAO == 0)
    {
        float cubeVertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindVertexArray(0); // 解绑VAO
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::CreatePrimitive(Geometry::Type type, const glm::vec3 &position, const glm::vec3 &scale,
                               const glm::vec3 &rotation, const Material &material)
{
    Geometry::Primitive primitive;
    primitive.type = type;
    primitive.position = position;
    primitive.scale = scale;
    primitive.rotation = rotation;

    switch (type)
    {
    case Geometry::SPHERE:
        primitive.mesh = Geometry::CreateSphere();
        primitive.params = {
            .sphere = {1.0f, 16} // 默认半径和分段数
        };
        break;
    case Geometry::CUBE:
        primitive.mesh = Geometry::CreateCube();
        primitive.params = {
            .cube = {1.0f, 1.0f, 1.0f} // 默认宽度、高度和深度
        };
        break;
    case Geometry::CYLINDER:
        primitive.mesh = Geometry::CreateCylinder();
        primitive.params = {
            .cylinder = {0.5f, 1.0f, 32} // 默认半径、高度和分段数
        };
        break;
    case Geometry::CONE:
        primitive.mesh = Geometry::CreateCone();
        primitive.params = {
            .cone = {0.5f, 1.0f, 32} // 默认半径、高度和分段数
        };
        break;
    case Geometry::PRISM:
        primitive.mesh = Geometry::CreatePrism();
        primitive.params = {
            .prism = {6, 0.5f, 1.0f} // 默认六边形，半径和高度
        };
        break;
    case Geometry::PYRAMID:
        primitive.mesh = Geometry::CreatePyramid();
        primitive.params = {
            .pyramid = {4, 0.5f, 1.0f} // 默认四边形金字塔，半径和高度
        };
        break;
    case Geometry::TORUS:
        primitive.mesh = Geometry::CreateTorus();
        primitive.params = {
            .torus = {1.0f, 0.3f, 48, 12} // 默认大半径、小半径、环段数和管段数
        };
        break;
    case Geometry::ELLIPSOID:
        primitive.mesh = Geometry::CreateEllipsoid();
        primitive.params = {
            .ellipsoid = {1.0f, 0.8f, 1.2f, 32} // 默认半径和分段数
        };
        break;
    case Geometry::FRUSTUM:
        primitive.mesh = Geometry::CreateFrustum();
        primitive.params = {
            .frustum = {0.5f, 0.5f, 1.0f, 32} // 默认上底半径、下底半径、高度和分段数
        };
        break;
    case Geometry::ARROW:
        primitive.mesh = Geometry::CreateArrow(1.0f, 0.2f);
        primitive.params = {
            .arrow = {1.0f, 0.2f} // 默认长度和箭头头部大小
        };
        break;
    default:
        std::cerr << "Unsupported geometry type!" << std::endl;
        return;
    }

    primitive.mesh->SetTransform(position, rotation, scale);
    primitive.mesh->SetMaterial(std::make_shared<Material>(material));

    primitives.push_back(primitive);
}

void Renderer::LoadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
{
    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath);
    shaders[name] = shader;
}

std::shared_ptr<Shader> Renderer::GetShader(const std::string &name) const
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        return it->second;
    }
    return nullptr;
}

void Renderer::UseShader(const std::string &name)
{
    auto shader = GetShader(name);
    if (shader)
    {
        shader->Use();
    }
    else
    {
        std::cerr << "Shader not found: " << name << std::endl;
    }
}

void Renderer::UseShader(const std::shared_ptr<Shader> &shader)
{
    if (shader)
    {
        shader->Use();
    }
    else
    {
        std::cerr << "Shader is null!" << std::endl;
    }
}
void Renderer::SetGlobalUniforms(const Camera &camera)
{
    forwardShader->SetMat4("view", camera.GetViewMatrix());
    forwardShader->SetMat4("projection", camera.GetProjectionMatrix(static_cast<float>(width) / height));
    forwardShader->SetVec3("viewPos", camera.Position);
}

void Renderer::RenderPostProcessing()
{
    if (msaaEnabled)
    {
        hdrBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        postShaderMS->Use();
        postShaderMS->SetInt("uSamples", msaaSamples); // 使用实际的采样数
        hdrBufferMS->BindTexture(0, 0); // 绑定多重采样的 HDR 纹理
        RenderQuad();
    }

    if (bloomEnabled)
    {
        bloomPrefilterBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        bloomPreShader->Use();
        bloomPreShader->SetFloat("threshold", 1.0f);
        hdrBuffer->BindTexture(0, 0); // scene
        RenderQuad();

        // Bloom Blur (Ping-Pong)
        bool horizontal = true;
        for (int i = 0; i < 10; ++i)
        {
            bloomBlurBuffers[horizontal]->Bind();
            bloomBlurShader->Use();
            bloomBlurShader->SetBool("horizontal", horizontal);
            (i == 0 ? bloomPrefilterBuffer : bloomBlurBuffers[!horizontal])->BindTexture(0, 0);
            RenderQuad();
            horizontal = !horizontal;
        }
    }

    if (fxaaEnabled)
    {
        fxaaBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        fxaaShader->Use();
        fxaaShader->SetInt("screenTexture", 0);
        fxaaShader->SetVec2("resolution", glm::vec2(width, height));
        hdrBuffer->BindTexture(0, 0);
        RenderQuad();
    }

    // Final Compose to viewport buffer (for ImGui)
    viewportBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    postProcessShader->Use();
    postProcessShader->SetBool("hdrEnabled", hdrEnabled);
    postProcessShader->SetBool("bloomEnabled", bloomEnabled);
    postProcessShader->SetBool("gammaEnabled", gammaCorrection);
    postProcessShader->SetFloat("exposure", 1.0f);

    // 绑定 HDR、Bloom、SSAO 纹理
    postProcessShader->SetInt("scene", 0);
    postProcessShader->SetInt("bloom", 1);
    if (fxaaEnabled)
        fxaaBuffer->BindTexture(0, 0);
    else
        hdrBuffer->BindTexture(0, 0);
    if (bloomEnabled)
        bloomBlurBuffers[false]->BindTexture(0, 1);

    RenderQuad();
    
    // Copy viewport buffer to default framebuffer (for window display)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, viewportBuffer->GetID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::shared_ptr<Model> Renderer::LoadModel(const std::string &path)
{
    auto model = std::make_shared<Model>(path);
    models.push_back(model);
    return model;
}

void Renderer::AddLight(const std::shared_ptr<Light> &light)
{
    if (std::dynamic_pointer_cast<PointLight>(light))
    {
        pointLights.push_back(std::dynamic_pointer_cast<PointLight>(light));
    }
    else if (std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        directionalLights.push_back(std::dynamic_pointer_cast<DirectionalLight>(light));
    }
    else if (std::dynamic_pointer_cast<SpotLight>(light))
    {
        spotLights.push_back(std::dynamic_pointer_cast<SpotLight>(light));
    }
}

void Renderer::SetGammaCorrection(bool enabled)
{
    gammaCorrection = enabled;
}

void Renderer::SetMSAA(bool enabled, int samples)
{
    if (msaaEnabled == enabled && msaaSamples == samples) {
        return; // 没有变化，直接返回
    }
    
    bool previousEnabled = msaaEnabled;
    int previousSamples = msaaSamples;
    
    msaaEnabled = enabled;
    msaaSamples = samples;
    
    if (enabled)
    {
        // 检查硬件支持的最大采样数
        GLint maxSamples;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        
        if (samples > maxSamples) {
            std::cerr << "Warning: Requested MSAA " << samples << "x exceeds hardware limit " 
                      << maxSamples << "x, using maximum supported." << std::endl;
            samples = maxSamples;
            msaaSamples = samples;
        }
        
        glEnable(GL_MULTISAMPLE);
        
        // 重新创建多重采样帧缓冲
        if (hdrBufferMS) {
            // 删除旧的多重采样缓冲
            hdrBufferMS.reset();
        }
        
        // 创建新的多重采样帧缓冲
        try {
            hdrBufferMS = std::make_unique<Framebuffer>(width, height);
            hdrBufferMS->AddColorTextureMultisample(GL_RGBA16F, samples);
            hdrBufferMS->AddDepthBufferMultisample(samples);
            hdrBufferMS->CheckComplete();
            
            std::cout << "MSAA " << samples << "x enabled successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to create MSAA framebuffer: " << e.what() << std::endl;
            // 恢复到之前的状态
            msaaEnabled = previousEnabled;
            msaaSamples = previousSamples;
            glDisable(GL_MULTISAMPLE);
            return;
        }
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
        std::cout << "MSAA disabled." << std::endl;
    }
}

void Renderer::SetHDR(bool enabled)
{
    hdrEnabled = enabled;
}

void Renderer::SetBloom(bool enabled)
{
    bloomEnabled = enabled;
}

void Renderer::SetSSAO(bool enabled)
{
    ssaoEnabled = enabled;
}

void Renderer::SetShadow(bool enabled)
{
    shadowEnabled = enabled;
}

// 多光源阴影管理函数实现
void Renderer::ClearLightShadowBuffers()
{
    lightShadowBuffers.clear();
    lightToShadowMap.clear();
}

void Renderer::CreateShadowBufferForLight(Light* light)
{
    // 实现为每个光源创建独立的阴影缓冲区
    // 这个函数可以根据需要进行扩展
}

void Renderer::RemoveShadowBufferForLight(Light* light)
{
    // 实现移除特定光源的阴影缓冲区
    // 这个函数可以根据需要进行扩展
}

unsigned int Renderer::GetShadowBufferIndexForLight(Light* light)
{
    // 实现获取光源对应的阴影缓冲区索引
    // 这个函数可以根据需要进行扩展
    return 0;
}

void Renderer::SetIBL(bool enabled)
{
    iblEnabled = enabled;
}

void Renderer::SetBackgroundType(BackgroundType type)
{
    backgroundType = type;
}

void Renderer::SetRenderMode(RenderMode mode)
{
    currentMode = mode;
}

void Renderer::Resize(int newWidth, int newHeight)
{
    if (width == newWidth && height == newHeight)
        return; // 避免重复

    // 1. 解绑所有当前绑定的 FBO，防止驱动还在用
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, newWidth, newHeight);

    width = newWidth;
    height = newHeight;

    // 更新帧缓冲和着色器
    gBuffer->Resize(newWidth, newHeight);
    hdrBuffer->Resize(newWidth, newHeight);
    hdrBufferMS->Resize(newWidth, newHeight);
    bloomBlurBuffers[0]->Resize(newWidth, newHeight);
    bloomBlurBuffers[1]->Resize(newWidth, newHeight);
    bloomPrefilterBuffer->Resize(newWidth, newHeight);
    ssaoBuffer->Resize(newWidth, newHeight);
    ssaoBlurBuffer->Resize(newWidth, newHeight);
    shadowBuffer->Resize(2048, 2048); // 阴影缓冲大小固定
    fxaaBuffer->Resize(newWidth, newHeight);
    viewportBuffer->Resize(newWidth, newHeight);
}

// Renderer.cpp
void Renderer::GenerateSSAOKernel()
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    ssaoKernel.clear();
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator) // 在半球内采样
        );

        // 标准化并缩放到0.0-1.0范围
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        // 使样本分布更接近原点
        float scale = static_cast<float>(i) / ssaoKernelSize;
        scale = 0.1f + 0.9f * scale * scale;
        sample *= scale;

        ssaoKernel.push_back(sample);
    }
}

void Renderer::GenerateSSAONoiseTexture()
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < ssaoNoiseSize * ssaoNoiseSize; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                        0.0f // 旋转在xy平面
        );
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &ssaoNoiseTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ssaoNoiseSize, ssaoNoiseSize, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// IBL相关实现
void Renderer::SetupIBL()
{
    // 创建IBL专用的帧缓冲
    iblCaptureBuffer = std::make_unique<Framebuffer>(512, 512);
    iblCaptureBuffer->AddDepthBuffer();
    iblCaptureBuffer->CheckComplete();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // 生成BRDF LUT纹理
    glGenTextures(1, &brdfLUTTexture);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 生成BRDF LUT
    iblCaptureBuffer->Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
    glViewport(0, 0, 512, 512);
    brdfShader->Use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    iblCaptureBuffer->Unbind();

    // 加载默认HDR环境贴图（如果存在）
    std::string hdrPath = FileSystem::GetPath("resources/textures/hdr/newport_loft.hdr");
    if (std::ifstream(hdrPath))
    {
        LoadHDREnvironment(hdrPath);
    }
    else
    {
        GenerateIBLTextures();
    }
}

void Renderer::LoadHDREnvironment(const std::string& hdrPath)
{
    // 保存当前viewport和帧缓冲设置
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLint prevFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
    
    // 加载HDR纹理
    unsigned int hdrTexture = LoadHDRTexture(hdrPath);
    
    if (hdrTexture == 0)
    {
        std::cerr << "Failed to load HDR texture: " << hdrPath << std::endl;
        return;
    }

    // 创建立方体贴图
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 设置投影和视图矩阵用于渲染立方体贴图
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = 
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +X
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // -X
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)), // +Y
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)), // -Y
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)), // +Z
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))  // -Z
    };

    // 转换HDR equirectangular到立方体贴图
    equirectangularToCubemapShader->Use();
    GLenum shaderError = glGetError();
    if (shaderError != GL_NO_ERROR) {
        std::cerr << "Error using equirectangularToCubemapShader: " << shaderError << std::endl;
    }
    
    equirectangularToCubemapShader->SetInt("equirectangularMap", 0);
    equirectangularToCubemapShader->SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);
    iblCaptureBuffer->Bind();

    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader->SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0);
    }

    iblCaptureBuffer->Unbind();

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after generating envCubemap: " << error << std::endl;
    }

    // 生成mipmaps
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // 生成辐照度贴图
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 调整IBL缓冲区尺寸为32x32用于辐照度贴图
    iblCaptureBuffer->Resize(32, 32);
    iblCaptureBuffer->Bind();

    irradianceShader->Use();
    irradianceShader->SetInt("environmentMap", 0);
    irradianceShader->SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader->SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0);
    }
    iblCaptureBuffer->Unbind();

    // 生成预过滤贴图
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    prefilterShader->Use();
    prefilterShader->SetInt("environmentMap", 0);
    prefilterShader->SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        iblCaptureBuffer->Resize(mipWidth, mipHeight);
        iblCaptureBuffer->Bind();
        
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->SetFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader->SetMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mip);
        }
        iblCaptureBuffer->Unbind();
    }

    // 清理
    glDeleteTextures(1, &hdrTexture);
    
    // 恢复原始设置
    glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    
    std::cout << "LoadHDREnvironment completed. envCubemap ID: " << envCubemap << std::endl;
}

unsigned int Renderer::LoadHDRTexture(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);  // 改为false避免翻转
    int width, height, nrComponents;
    float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture = 0;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image: " << path << std::endl;
    }

    return hdrTexture;
}

void Renderer::GenerateIBLTextures()
{
    // 使用现有的天空盒立方体贴图作为环境贴图
    envCubemap = environmentMap->GetID();
    
    // 生成辐照度贴图和预过滤贴图（基于现有环境贴图）
    // 这里可以添加基于现有立方体贴图生成IBL纹理的代码
    // 为简化，我们暂时使用默认值
    irradianceMap = envCubemap;
    prefilterMap = envCubemap;
}