# Details

Date : 2025-08-05 15:06:13

Directory d:\\Adata\\study\\opengl\\engine

Total : 56 files,  13497 codes, 2050 comments, 2128 blanks, all 17675 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [imgui.ini](/imgui.ini) | Ini | 21 | 0 | 7 | 28 |
| [include/core/Application.hpp](/include/core/Application.hpp) | C++ | 30 | 0 | 10 | 40 |
| [include/core/Camera.hpp](/include/core/Camera.hpp) | C++ | 58 | 4 | 14 | 76 |
| [include/core/Framebuffer.hpp](/include/core/Framebuffer.hpp) | C++ | 62 | 13 | 22 | 97 |
| [include/core/Geometry.hpp](/include/core/Geometry.hpp) | C++ | 140 | 1 | 7 | 148 |
| [include/core/Light.hpp](/include/core/Light.hpp) | C++ | 175 | 9 | 38 | 222 |
| [include/core/Material.hpp](/include/core/Material.hpp) | C++ | 40 | 3 | 11 | 54 |
| [include/core/Mesh.hpp](/include/core/Mesh.hpp) | C++ | 76 | 3 | 14 | 93 |
| [include/core/Model.hpp](/include/core/Model.hpp) | C++ | 61 | 0 | 12 | 73 |
| [include/core/Renderer.hpp](/include/core/Renderer.hpp) | C++ | 268 | 21 | 41 | 330 |
| [include/core/Shader.hpp](/include/core/Shader.hpp) | C++ | 29 | 1 | 8 | 38 |
| [include/core/Texture.hpp](/include/core/Texture.hpp) | C++ | 39 | 1 | 6 | 46 |
| [include/stb\_image.h](/include/stb_image.h) | C++ | 5,833 | 1,294 | 861 | 7,988 |
| [include/ui/EditorUI.hpp](/include/ui/EditorUI.hpp) | C++ | 124 | 18 | 24 | 166 |
| [include/utils/FileDialog.hpp](/include/utils/FileDialog.hpp) | C++ | 8 | 5 | 3 | 16 |
| [include/utils/FileSystem.hpp](/include/utils/FileSystem.hpp) | C++ | 10 | 0 | 2 | 12 |
| [include/utils/Logger.hpp](/include/utils/Logger.hpp) | C++ | 27 | 0 | 7 | 34 |
| [resources/scene/test.json](/resources/scene/test.json) | JSON | 370 | 0 | 0 | 370 |
| [resources/scene/test\_scene.json](/resources/scene/test_scene.json) | JSON | 350 | 0 | 0 | 350 |
| [resources/shaders/deferred/geometry\_pass.frag](/resources/shaders/deferred/geometry_pass.frag) | GLSL | 66 | 6 | 11 | 83 |
| [resources/shaders/deferred/geometry\_pass.vert](/resources/shaders/deferred/geometry_pass.vert) | GLSL | 23 | 0 | 6 | 29 |
| [resources/shaders/deferred/lighting\_pass.frag](/resources/shaders/deferred/lighting_pass.frag) | GLSL | 152 | 43 | 46 | 241 |
| [resources/shaders/deferred/lighting\_pass.vert](/resources/shaders/deferred/lighting_pass.vert) | GLSL | 15 | 1 | 2 | 18 |
| [resources/shaders/forward/blinn\_phong.frag](/resources/shaders/forward/blinn_phong.frag) | GLSL | 197 | 39 | 63 | 299 |
| [resources/shaders/forward/blinn\_phong.vert](/resources/shaders/forward/blinn_phong.vert) | GLSL | 23 | 0 | 6 | 29 |
| [resources/shaders/postprocess/bloom\_blur.frag](/resources/shaders/postprocess/bloom_blur.frag) | GLSL | 28 | 0 | 2 | 30 |
| [resources/shaders/postprocess/bloom\_prefilter.frag](/resources/shaders/postprocess/bloom_prefilter.frag) | GLSL | 14 | 0 | 2 | 16 |
| [resources/shaders/postprocess/fxaa.frag](/resources/shaders/postprocess/fxaa.frag) | GLSL | 46 | 9 | 13 | 68 |
| [resources/shaders/postprocess/post.frag](/resources/shaders/postprocess/post.frag) | GLSL | 22 | 3 | 6 | 31 |
| [resources/shaders/postprocess/post\_msaa.frag](/resources/shaders/postprocess/post_msaa.frag) | GLSL | 16 | 0 | 3 | 19 |
| [resources/shaders/postprocess/quad.vert](/resources/shaders/postprocess/quad.vert) | GLSL | 9 | 0 | 3 | 12 |
| [resources/shaders/postprocess/ssao.frag](/resources/shaders/postprocess/ssao.frag) | GLSL | 38 | 8 | 12 | 58 |
| [resources/shaders/postprocess/ssao\_blur.frag](/resources/shaders/postprocess/ssao_blur.frag) | GLSL | 15 | 1 | 4 | 20 |
| [resources/shaders/utility/light.frag](/resources/shaders/utility/light.frag) | GLSL | 7 | 0 | 2 | 9 |
| [resources/shaders/utility/light.vert](/resources/shaders/utility/light.vert) | GLSL | 9 | 0 | 2 | 11 |
| [resources/shaders/utility/shadow\_depth.frag](/resources/shaders/utility/shadow_depth.frag) | GLSL | 4 | 2 | 1 | 7 |
| [resources/shaders/utility/shadow\_depth.vert](/resources/shaders/utility/shadow_depth.vert) | GLSL | 12 | 0 | 3 | 15 |
| [resources/shaders/utility/skybox.frag](/resources/shaders/utility/skybox.frag) | GLSL | 13 | 1 | 5 | 19 |
| [resources/shaders/utility/skybox.vert](/resources/shaders/utility/skybox.vert) | GLSL | 12 | 2 | 3 | 17 |
| [src/core/Application.cpp](/src/core/Application.cpp) | C++ | 228 | 20 | 38 | 286 |
| [src/core/Camera.cpp](/src/core/Camera.cpp) | C++ | 81 | 1 | 14 | 96 |
| [src/core/Framebuffer.cpp](/src/core/Framebuffer.cpp) | C++ | 167 | 3 | 35 | 205 |
| [src/core/Geometry.cpp](/src/core/Geometry.cpp) | C++ | 519 | 85 | 134 | 738 |
| [src/core/Light.cpp](/src/core/Light.cpp) | C++ | 365 | 80 | 69 | 514 |
| [src/core/Material.cpp](/src/core/Material.cpp) | C++ | 76 | 0 | 12 | 88 |
| [src/core/Mesh.cpp](/src/core/Mesh.cpp) | C++ | 49 | 7 | 16 | 72 |
| [src/core/Model.cpp](/src/core/Model.cpp) | C++ | 226 | 17 | 37 | 280 |
| [src/core/Renderer.cpp](/src/core/Renderer.cpp) | C++ | 1,540 | 153 | 207 | 1,900 |
| [src/core/Shader.cpp](/src/core/Shader.cpp) | C++ | 118 | 3 | 26 | 147 |
| [src/core/Texture.cpp](/src/core/Texture.cpp) | C++ | 126 | 1 | 18 | 145 |
| [src/main.cpp](/src/main.cpp) | C++ | 8 | 1 | 1 | 10 |
| [src/ui/EditorUI.cpp](/src/ui/EditorUI.cpp) | C++ | 1,426 | 114 | 208 | 1,748 |
| [src/utils/FileDialog.cpp](/src/utils/FileDialog.cpp) | C++ | 48 | 4 | 17 | 69 |
| [src/utils/FileSystem.cpp](/src/utils/FileSystem.cpp) | C++ | 23 | 2 | 5 | 30 |
| [src/utils/Logger.cpp](/src/utils/Logger.cpp) | C++ | 37 | 1 | 6 | 44 |
| [xmake.lua](/xmake.lua) | XMake | 18 | 70 | 3 | 91 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)