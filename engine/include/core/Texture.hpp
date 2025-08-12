#pragma once

#include <glm/glm.hpp>
#include <string>
#include <glad/glad.h>
#include <vector>

class Texture
{
  public:
    Texture();
    ~Texture();
    explicit Texture(const std::string &filePath) : Texture() { 
        LoadFromFile(filePath); 
    }
    bool LoadFromFile(const std::string &filepath);
    void Generate(unsigned int width, unsigned int height, unsigned char *data);
    void Bind(unsigned int unit = 0) const;

    void CreateSolidColor(const glm::vec3 &color);
    void CreateNormalMap();
    void LoadCubemap(const std::vector<std::string> &faces);
    void CreateCubemap(unsigned int width, unsigned int height, unsigned int internalFormat = GL_RGB16F);

    unsigned int GetID() const
    {
        return ID;
    }
    const std::string &GetPath() const
    {
        return Path;
    }
    std::string type;
    bool flipY = true;
    
    // 纹理ID管理
    static unsigned int GetNextTextureID();

  private:
    unsigned int ID;
    int Width, Height, nrComponents;
    std::string Path;

    // 纹理参数
    unsigned int Internal_Format;
    unsigned int Image_Format;
    unsigned int Wrap_S;
    unsigned int Wrap_T;
    unsigned int Filter_Min;
    unsigned int Filter_Max;
    
    // 静态ID计数器，从较高的数字开始以避免与ImGui冲突
    static unsigned int nextTextureID;
};