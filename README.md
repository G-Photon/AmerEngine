# AmerEngine

![AmerEngine Logo](https://img.shields.io/badge/OpenGL-Engine-blue.svg)


一个基于 OpenGL 的现代渲染引擎，仿 Unity 编辑器风格，支持：
- 多种简单几何体（球体、长方体、圆柱体、圆锥体、棱柱、棱锥、正多面体、圆环体、椭球）
- 外部模型导入（OBJ/PMX等，自动/手动贴图绑定）
- 多种光源（点光源、定向光、聚光灯）
- 摄像机自由操控（右键+WASD/鼠标）
- 材质参数可调（Blinn-Phong/PBR，支持多种贴图）
- 丰富渲染特效（MSAA抗锯齿、Gamma校正、阴影、法线/视差贴图、HDR、泛光、延迟着色、SSAO、IBL等）
- 场景管理与序列化（JSON）
- ImGui 编辑器界面，所见即所得
- 资源自动管理（shaders/textures/models）

---

## 🚀 快速开始

### 依赖环境
- C++20
- [xmake](https://xmake.io/)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [Assimp](https://github.com/assimp/assimp)
- [ImGui (Docking)](https://github.com/ocornut/imgui)
- [nlohmann_json](https://github.com/nlohmann/json)
- [libsdl3](https://github.com/libsdl-org/SDL)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)

### 构建与运行
```sh
# 1. 安装依赖（xmake 会自动拉取）
cd ./engine
xmake

# 2. 运行
xmake run AmerEngine
```

> **注意**：首次编译需联网以自动下载依赖。

---

## 📋 Todo List（欢迎补充/认领）
- [ ] 支持更多后处理特效（如Bloom、SSR等）
- [ ] 完善动画系统
- [ ] 场景资源热加载
- [ ] 跨平台打包与部署
- [ ] 完善文档与API注释
- [ ] 单元测试与CI集成
- [ ] 支持 PBR 材质

---

## ✅ 已完成功能（持续完善中）
- [x] 场景管理（新建、保存、加载）
- [x] 支持 Blinn-Phong 材质
- [x] 支持 OBJ/PMX 等模型加载
- [x] 支持多光源与阴影
- [x] ImGui 编辑器界面
- [x] 通用通知系统（UI 反馈）
- [x] JSON 场景序列化
- [x] 资源自动拷贝与路径兼容
- [x] MSAA、FXAA抗锯齿方法
- [x] Gamma校正、HDR、泛光、SSAO等后处理效果
- [x] 延迟着色

---

## 🌟 主要特性一览

| 功能类别 | 说明 |
|---|---|
| 支持几何体 | 球体、长方体、圆柱体、圆锥体、棱柱、棱锥、正多面体、圆环体、椭球 |
| 外部模型 | OBJ/PMX 导入，自动/手动贴图绑定 |
| 光源类型 | 点光源、定向光、聚光灯，参数可调 |
| 材质系统 | Blinn-Phong、PBR，支持多种贴图（漫反射、镜面、法线、金属、粗糙度、AO等） |
| 渲染特效 | MSAA、Gamma校正、阴影、法线/视差贴图、HDR、泛光、延迟着色、SSAO、IBL |
| 编辑器 | ImGui 界面，支持场景层级、属性面板、资源浏览、通知系统 |
| 参数可调 | 所有模型/光源/材质参数均可实时调整 |
| 资源管理 | 统一的shaders/textures/models路径，自动拷贝 |
| 场景管理 | 新建、保存、加载，JSON序列化 |

```
AmerEngine/
├── engine/
│   ├── src/           # 源码实现（core/ geometry/ ui/ utils/）
│   ├── include/       # 头文件（与src结构对应）
│   ├── resources/
│   │   ├── shaders/   # GLSL着色器
│   │   ├── textures/  # 贴图
│   │   └── models/    # 外部模型
│   ├── build/         # 构建输出
│   ├── xmake.lua      # 构建脚本
└── README.md      # 项目说明
```

---

## 📝 贡献指南
欢迎 PR 和 Issue！请遵循 C++ 代码风格，提交前请确保能通过编译。

---

## 📞 联系方式
- 作者: G-Photon
- 邮箱: aucafy@gmail.com

---

> 本项目仅供学习与交流，部分资源来源于网络，版权归原作者所有。
