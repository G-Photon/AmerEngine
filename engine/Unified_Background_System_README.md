# 统一背景管理系统 - 重构版本

## 概述

经过重构，原本分离的"环境贴图"和"背景"概念已经统一为单一的**背景管理系统**。这个统一的系统同时处理：
- HDR环境贴图
- 传统天空盒立方体贴图
- 所有相关的IBL纹理生成

## 🔄 重构内容

### 1. 概念统一
- **之前**：环境贴图和背景是两个分离的概念
- **现在**：统一为"背景"概念，包含两种类型：
  - `HDR_ENVIRONMENT`：HDR环境贴图
  - `SKYBOX_CUBEMAP`：传统6面天空盒

### 2. 数据结构重构

#### 新的BackgroundData结构
```cpp
struct BackgroundData {
    std::string name;           // 背景名称
    std::string path;           // 文件路径
    BackgroundType type;        // 背景类型
    unsigned int envCubemap = 0;     // 环境立方体贴图
    unsigned int irradianceMap = 0;  // 辐照度贴图
    unsigned int prefilterMap = 0;   // 预过滤贴图
    bool isHDR = false;         // 是否为HDR格式
};
```

### 3. API接口重构

#### 统一的公共接口
```cpp
// 加载HDR环境背景
void LoadBackgroundEnvironment(const std::string& name, const std::string& path, bool isHDR = true);

// 加载天空盒背景
void LoadBackgroundSkybox(const std::string& name, const std::vector<std::string>& faces);

// 切换背景
void SwitchBackground(const std::string& name);

// 获取可用背景列表
std::vector<std::string> GetAvailableBackgrounds() const;

// 获取当前背景名称
std::string GetCurrentBackgroundName() const;
```

### 4. UI界面重构

#### 更新后的界面组织
在"渲染器设置" → "光照" → "背景管理"中提供：

1. **当前背景信息**
   - 显示当前背景名称
   - 显示背景类型（HDR Environment 或 Skybox Cubemap）

2. **背景选择器**
   - 统一的下拉菜单选择所有类型的背景
   - 实时切换不同背景

3. **添加新背景**
   - "添加HDR背景"按钮：支持.hdr文件
   - "添加天空盒背景"按钮：支持6面立方体贴图（待实现）

4. **背景详情**
   - 显示所有可用背景的统一列表
   - 点击切换功能

## 🎯 功能特性

### ✅ 已实现功能

1. **统一管理**
   - 所有背景类型在单一系统中管理
   - 自动IBL纹理生成（irradianceMap + prefilterMap）
   - 智能资源管理和清理

2. **类型自动识别**
   - HDR文件自动识别为环境贴图类型
   - 天空盒自动识别为立方体贴图类型
   - 渲染器根据类型自动调整处理方式

3. **无缝切换**
   - 运行时在不同背景间无缝切换
   - 自动更新IBL光照
   - 保持渲染性能

4. **智能命名**
   - 自动避免重名冲突
   - 支持自定义背景名称

### 🔧 技术实现

1. **IBL管线统一**
   ```
   背景源 → 立方体贴图 → 辐照度贴图 → 预过滤贴图
   ```

2. **资源管理**
   - 统一的`std::unordered_map<std::string, BackgroundData> backgrounds`
   - 自动OpenGL纹理资源清理
   - 内存泄漏预防

3. **类型安全**
   - `BackgroundType`枚举确保类型安全
   - 编译时类型检查

## 📋 使用指南

### 基本操作

1. **查看当前背景**
   - 启动程序自动加载默认背景：
     - "Newport Loft HDR" (HDR Environment)
     - "Default Skybox" (Skybox Cubemap)

2. **切换背景**
   ```
   渲染器设置 → 光照 → 背景管理 → 选择背景
   ```

3. **添加新HDR背景**
   ```
   背景管理 → 添加HDR背景 → 选择.hdr文件
   ```

### 高级功能

1. **程序化背景管理**
   ```cpp
   // 加载新的HDR背景
   renderer->LoadBackgroundEnvironment("Sunset Beach", "path/to/sunset.hdr");
   
   // 切换到指定背景
   renderer->SwitchBackground("Sunset Beach");
   
   // 获取所有可用背景
   auto backgrounds = renderer->GetAvailableBackgrounds();
   ```

2. **背景类型查询**
   ```cpp
   auto type = renderer->GetBackgroundType();
   if (type == Renderer::HDR_ENVIRONMENT) {
       // 当前使用HDR环境贴图
   }
   ```

## 🚀 性能优化

1. **预计算IBL纹理**：所有IBL纹理在加载时一次性生成
2. **智能缓存**：避免重复生成相同的纹理
3. **资源复用**：多个背景可以共享相同的IBL管线
4. **内存优化**：自动清理未使用的纹理资源

## 🔮 未来扩展

1. **天空盒文件夹导入**
   - 支持自动识别6面文件的文件夹
   - 批量导入多个天空盒

2. **背景预览**
   - UI中显示背景缩略图
   - 实时预览球体

3. **高级参数控制**
   - 背景强度调节
   - 旋转和偏移控制
   - HDR曝光调节

4. **背景动画**
   - 支持天空盒动画
   - 背景过渡效果

## 📁 文件结构

```
resources/
├── textures/
│   ├── hdr/
│   │   └── newport_loft.hdr          # HDR环境背景
│   └── skybox/                       # 天空盒背景
│       ├── right.jpg
│       ├── left.jpg
│       ├── top.jpg
│       ├── bottom.jpg
│       ├── front.jpg
│       └── back.jpg
└── shaders/
    └── ibl/                          # 统一的IBL着色器
        ├── cubemap.vert
        ├── equirectangular_to_cubemap.frag
        ├── irradiance_convolution.frag
        ├── prefilter.frag
        └── brdf_lut.frag
```

## ✨ 总结

通过这次重构，我们实现了：

1. **概念统一**：环境贴图和背景不再是两个分离的概念
2. **API简化**：更清晰、更直观的接口设计
3. **管理统一**：所有背景类型在单一系统中管理
4. **用户体验提升**：更直观的UI界面和操作流程
5. **代码质量提升**：减少重复代码，提高可维护性

这个统一的背景管理系统为引擎提供了强大而灵活的环境光照控制能力，同时保持了简洁的用户界面和清晰的概念模型。
