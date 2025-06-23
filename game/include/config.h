#pragma once
#include <glad/glad.h>
// ===== 基础配置 =====
#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 3
#define GLSL_VERSION "#version 330 core"


// ===== 纹理默认参数 =====
namespace GLDefaults
{
constexpr int TEXTURE_WRAP = GL_REPEAT;
constexpr int TEXTURE_FILTER_MIN = GL_LINEAR_MIPMAP_LINEAR;
constexpr int TEXTURE_FILTER_MAG = GL_LINEAR;
} // namespace GLDefaults

// ===== 默认常量 =====
#ifndef PATH_DIR
#define PATH_DIR "../../../../"
#endif

#ifndef RES_DIR
#define RES_DIR "../../../../resources/"
#endif