#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "core/Texture.hpp"

class TextureManager
{
public:
    static TextureManager& GetInstance();
    
    // 获取或加载纹理
    std::shared_ptr<Texture> GetTexture(const std::string& path);
    
    // 获取或加载纹理（带flipY参数）
    std::shared_ptr<Texture> GetTexture(const std::string& path, bool flipY);
    
    // 清理所有纹理缓存
    void ClearCache();
    
    // 获取缓存统计信息
    size_t GetCacheSize() const;
    
private:
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    
    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
};
