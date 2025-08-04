# 延迟渲染阴影支持

## 修改内容

### 1. 延迟渲染着色器修改 (`lighting_pass.frag`)

#### 新增功能
- 在光源结构体 `L` 中添加阴影相关字段：
  - `bool hasShadows`: 是否启用阴影
  - `sampler2D shadowMap`: 阴影贴图
  - `mat4 lightSpaceMatrix`: 光源空间变换矩阵
- 添加全局阴影开关 `uniform bool shadowEnabled`
- 实现 `ShadowCalculation()` 函数，支持PCF软阴影

#### 光照计算修改
- **方向光** (`calculateDirectionalLight`): 添加阴影计算，阴影应用到漫反射和镜面反射
- **点光源** (`calculatePointLight`): 添加阴影计算，考虑距离衰减
- **聚光灯** (`calculateSpotLight`): 添加阴影计算，考虑聚光强度和距离衰减

### 2. Light.cpp 中的 drawLightMesh 方法修改

#### 统一阴影设置
为所有光源类型的 `drawLightMesh` 方法添加阴影参数设置：
- 设置 `light.hasShadows` uniform
- 绑定阴影贴图到纹理单元30（延迟渲染专用）
- 设置光源空间变换矩阵
- 配置阴影贴图纹理参数（边界处理等）

#### 修改的光源类型
1. **DirectionalLight::drawLightMesh()**
2. **PointLight::drawLightMesh()**  
3. **SpotLight::drawLightMesh()**

### 3. Renderer.cpp 中的延迟渲染修改

#### RenderDeferred() 函数
- 添加全局阴影开关设置：`deferredLightingShader->SetBool("shadowEnabled", shadowEnabled)`
- 确保在渲染每个光源时正确传递阴影信息

## 技术特点

### 阴影计算优化
- **PCF软阴影**: 使用3x3采样核心减少阴影锯齿
- **边界处理**: 超出阴影贴图范围的区域不产生阴影
- **深度偏移**: 使用0.005的bias值减少阴影失真

### 纹理单元管理
- 延迟渲染使用纹理单元30专门绑定阴影贴图
- 避免与G-Buffer纹理（0-7）和SSAO纹理（8）冲突
- 前向渲染继续使用分类纹理单元（10+, 15+, 20+）

### 性能考虑
- 延迟渲染中每个光源独立渲染，自然支持多光源阴影
- 阴影计算只在启用阴影且有有效阴影贴图时执行
- 使用条件编译避免不必要的计算开销

## 使用说明

1. **启用全局阴影**: 确保 `Renderer::SetShadow(true)` 被调用
2. **启用光源阴影**: 对需要阴影的光源调用 `light->SetShadowEnabled(true)`
3. **模式切换**: 前向渲染和延迟渲染都支持多光源阴影
4. **性能监控**: 建议监控帧率，过多阴影光源可能影响性能

## 兼容性

- 保持与原有前向渲染阴影系统的兼容性
- 延迟渲染和前向渲染可以独立使用阴影功能
- EditorUI中的阴影控制对两种渲染模式都有效
