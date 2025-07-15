#pragma once

#include <glm/glm.hpp>
#include <string>
#include <glad/glad.h>

class Texture
{
  public:
    Texture();
    ~Texture();

    bool LoadFromFile(const std::string &filepath);
    void Generate(unsigned int width, unsigned int height, unsigned char *data);
    void Bind(unsigned int unit = 0) const;

    void CreateSolidColor(const glm::vec3 &color);
    void CreateNormalMap();

    unsigned int GetID() const
    {
        return ID;
    }
    const std::string &GetPath() const
    {
        return Path;
    }
    std::string type;

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
};