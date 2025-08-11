# 环境贴图管理系统

## 功能概述

本项目已实现了完整的IBL（基于图像的光照）环境贴图管理系统，支持：

1. **预生成环境贴图的IBL纹理**
   - 自动为现有的`newport_loft.hdr`生成irradianceMap和prefilterMap
   - 自动为`resources/textures/skybox`中的天空盒生成IBL纹理

2. **动态加载新环境贴图**
   - 支持用户通过UI上传新的HDR环境贴图
   - 自动生成对应的irradianceMap和prefilterMap
   - 支持实时切换不同的环境贴图

3. **环境贴图管理界面**
   - 在"渲染器设置"面板中提供环境贴图管理功能
   - 显示当前可用的环境贴图列表
   - 支持一键切换环境贴图

## 系统架构

### 核心类修改

#### Renderer.hpp/cpp
新增了以下主要功能：

```cpp
// 环境贴图数据结构
struct EnvironmentMapData {
    std::string name;
    std::string path;
    unsigned int envCubemap = 0;
    unsigned int irradianceMap = 0;
    unsigned int prefilterMap = 0;
    bool isHDR = false;
};

// 公共接口
void LoadEnvironmentMap(const std::string& name, const std::string& path, bool isHDR = true);
void SwitchEnvironmentMap(const std::string& name);
std::vector<std::string> GetAvailableEnvironmentMaps() const;
std::string GetCurrentEnvironmentMapName() const;
```

#### 主要方法

1. **InitializeDefaultEnvironments()**
   - 在初始化时自动加载默认环境贴图
   - 为Newport Loft HDR和默认天空盒生成IBL纹理

2. **LoadEnvironmentMap()**
   - 加载HDR环境贴图文件
   - 转换equirectangular贴图为立方体贴图
   - 生成IBL所需的irradianceMap和prefilterMap

3. **LoadSkyboxEnvironment()**
   - 加载传统6面天空盒
   - 生成对应的IBL纹理

4. **GenerateIBLForEnvironment()**
   - 为任意环境贴图生成IBL纹理
   - 创建32x32的irradianceMap用于漫反射光照
   - 创建128x128多级mipmap的prefilterMap用于镜面反射

5. **SwitchEnvironmentMap()**
   - 实时切换当前使用的环境贴图
   - 更新渲染器的IBL纹理引用

### UI界面修改

#### EditorUI.cpp
在"渲染器设置"→"光照"→"环境贴图管理"中新增：

- **当前环境贴图显示**：显示当前使用的环境贴图名称
- **环境贴图选择器**：下拉菜单选择不同的环境贴图
- **添加HDR环境贴图按钮**：打开文件对话框选择新的HDR文件
- **环境贴图详情**：显示所有可用环境贴图的列表，支持点击切换

## 使用方法

### 1. 查看现有环境贴图
启动程序后，系统会自动加载默认环境贴图：
- "Newport Loft HDR"：来自resources/textures/hdr/newport_loft.hdr
- "Default Skybox"：来自resources/textures/skybox/的6个面

### 2. 切换环境贴图
1. 打开"渲染器设置"面板
2. 展开"光照"→"环境贴图管理"
3. 在"选择环境贴图"下拉菜单中选择想要的环境贴图
4. 场景的IBL光照会立即更新

### 3. 添加新的HDR环境贴图
1. 在"环境贴图管理"中点击"添加HDR环境贴图"
2. 在文件对话框中选择.hdr格式的环境贴图文件
3. 系统会自动：
   - 加载HDR文件
   - 转换为立方体贴图格式
   - 生成irradianceMap（32x32）
   - 生成prefilterMap（128x128，5级mipmap）
   - 添加到可用环境贴图列表

### 4. 实时预览
- 所有环境贴图变更都会实时反映在场景中
- PBR材质的反射和环境光照会立即更新
- 支持在不同环境贴图间快速切换对比效果

## 技术细节

### IBL纹理生成流程

1. **立方体贴图生成**
   - HDR文件通过equirectangular到cubemap着色器转换
   - 生成512x512的立方体贴图，包含完整mipmap链

2. **辐照度贴图生成**
   - 使用irradiance_convolution着色器
   - 对环境贴图进行卷积采样
   - 生成32x32低分辨率用于漫反射光照

3. **预过滤贴图生成**
   - 使用prefilter着色器
   - 根据不同roughness值预过滤环境贴图
   - 生成5级mipmap（128→64→32→16→8）
   - 用于不同粗糙度材质的镜面反射

### 性能优化

- **预计算**：IBL纹理在加载时一次性生成，运行时无额外开销
- **内存管理**：自动清理旧的环境贴图资源
- **渲染优化**：保持原有渲染管线效率

## 支持的文件格式

- **HDR环境贴图**：.hdr格式（radiance格式）
- **天空盒**：传统6面立方体贴图（.jpg, .png等）

## 预设环境贴图

目前包含的环境贴图：
1. **Newport Loft HDR**：室内环境，适合展示金属反射效果
2. **Default Skybox**：传统天空盒，适合一般场景

## 扩展建议

1. **添加更多预设环境贴图**：
   - 室外HDR环境（如森林、海滩、城市）
   - 不同时间和天气的环境贴图

2. **环境贴图预览**：
   - 在UI中显示环境贴图缩略图
   - 支持环境贴图的实时预览球

3. **环境贴图参数调节**：
   - 环境贴图强度调节
   - 旋转和偏移参数

4. **批量导入**：
   - 支持一次性导入多个HDR文件
   - 环境贴图文件夹扫描

## 文件结构

```
resources/
├── textures/
│   ├── hdr/
│   │   └── newport_loft.hdr          # 默认HDR环境贴图
│   └── skybox/                       # 默认天空盒面
│       ├── right.jpg
│       ├── left.jpg
│       ├── top.jpg
│       ├── bottom.jpg
│       ├── front.jpg
│       └── back.jpg
└── shaders/
    └── ibl/                          # IBL相关着色器
        ├── cubemap.vert
        ├── equirectangular_to_cubemap.frag
        ├── irradiance_convolution.frag
        ├── prefilter.frag
        └── brdf_lut.frag
```

此系统为引擎提供了完整的环境贴图管理能力，支持实时切换和动态加载，大大增强了PBR渲染的灵活性和视觉效果。
