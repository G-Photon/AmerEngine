#include "core/Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

Texture::Texture()
    : ID(0), Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT),
      Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
{
    glGenTextures(1, &ID);
}

Texture::~Texture()
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
        ID = 0;
    }
}

void Texture::Generate(unsigned int width, unsigned int height, unsigned char *data)
{
    Width = width;
    Height = height;

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, Internal_Format, width, height, 0, Image_Format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

bool Texture::LoadFromFile(const std::string &filepath)
{
    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(filepath.c_str(), &Width, &Height, &nrComponents, 0);
    if (data)
    {
        if (nrComponents == 1)
        {
            Internal_Format = GL_RED;
            Image_Format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            Internal_Format = GL_RGB;
            Image_Format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            Internal_Format = GL_RGBA;
            Image_Format = GL_RGBA;
        }

        Generate(Width, Height, data);
        stbi_image_free(data);
        Path = filepath;
        return true;
    }

    std::cerr << "Texture failed to load at path: " << filepath << std::endl;
    stbi_image_free(data);
    return false;
}

void Texture::CreateSolidColor(const glm::vec3 &color)
{
    unsigned char data[3] = {static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255),
                             static_cast<unsigned char>(color.b * 255)};

    Internal_Format = GL_RGB;
    Image_Format = GL_RGB;
    Generate(1, 1, data);
}

void Texture::CreateNormalMap()
{
    unsigned char data[3] = {128, 128, 255}; // 默认法线(0,0,1)
    Internal_Format = GL_RGB;
    Image_Format = GL_RGB;
    Generate(1, 1, data);
}