#include "core/TextureManager.hpp"
#include <iostream>

TextureManager& TextureManager::GetInstance()
{
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& path)
{
    return GetTexture(path, true); // 默认翻转
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& path, bool flipY)
{
    if (path.empty()) {
        return nullptr;
    }
    
    // 创建包含flipY信息的缓存键
    std::string cacheKey = path + "_flip_" + (flipY ? "1" : "0");
    
    // Check if texture already exists in cache
    auto it = textureCache.find(cacheKey);
    if (it != textureCache.end()) {
        return it->second;
    }
    
    // Create new texture
    auto texture = std::make_shared<Texture>();
    texture->flipY = flipY; // 设置翻转状态
    
    if (texture->LoadFromFile(path)) {
        textureCache[cacheKey] = texture;
    } else {
        std::cerr << "Texture failed to load, not cached: " << path << std::endl;
        return nullptr;
    }
    
    return texture;
}

void TextureManager::ClearCache()
{
    textureCache.clear();
}

size_t TextureManager::GetCacheSize() const
{
    return textureCache.size();
}
