# Details

Date : 2025-07-24 19:38:28

Directory d:\\Adata\\study\\opengl\\engine

Total : 46 files,  10309 codes, 1710 comments, 1690 blanks, all 13709 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [include/core/Application.hpp](/include/core/Application.hpp) | C++ | 29 | 0 | 10 | 39 |
| [include/core/Camera.hpp](/include/core/Camera.hpp) | C++ | 58 | 4 | 14 | 76 |
| [include/core/Framebuffer.hpp](/include/core/Framebuffer.hpp) | C++ | 60 | 12 | 21 | 93 |
| [include/core/Geometry.hpp](/include/core/Geometry.hpp) | C++ | 140 | 1 | 7 | 148 |
| [include/core/Light.hpp](/include/core/Light.hpp) | C++ | 125 | 1 | 26 | 152 |
| [include/core/Material.hpp](/include/core/Material.hpp) | C++ | 43 | 3 | 11 | 57 |
| [include/core/Mesh.hpp](/include/core/Mesh.hpp) | C++ | 76 | 3 | 14 | 93 |
| [include/core/Model.hpp](/include/core/Model.hpp) | C++ | 52 | 0 | 11 | 63 |
| [include/core/Renderer.hpp](/include/core/Renderer.hpp) | C++ | 227 | 19 | 33 | 279 |
| [include/core/Shader.hpp](/include/core/Shader.hpp) | C++ | 29 | 1 | 8 | 38 |
| [include/core/Texture.hpp](/include/core/Texture.hpp) | C++ | 34 | 1 | 7 | 42 |
| [include/stb\_image.h](/include/stb_image.h) | C++ | 5,833 | 1,294 | 861 | 7,988 |
| [include/ui/EditorUI.hpp](/include/ui/EditorUI.hpp) | C++ | 60 | 2 | 11 | 73 |
| [include/utils/FileDialog.hpp](/include/utils/FileDialog.hpp) | C++ | 8 | 5 | 3 | 16 |
| [include/utils/FileSystem.hpp](/include/utils/FileSystem.hpp) | C++ | 10 | 0 | 2 | 12 |
| [include/utils/Logger.hpp](/include/utils/Logger.hpp) | C++ | 27 | 0 | 7 | 34 |
| [resources/shaders/deferred/geometry\_pass.frag](/resources/shaders/deferred/geometry_pass.frag) | GLSL | 49 | 6 | 11 | 66 |
| [resources/shaders/deferred/geometry\_pass.vert](/resources/shaders/deferred/geometry_pass.vert) | GLSL | 23 | 0 | 6 | 29 |
| [resources/shaders/deferred/lighting\_pass.frag](/resources/shaders/deferred/lighting_pass.frag) | GLSL | 123 | 18 | 38 | 179 |
| [resources/shaders/deferred/lighting\_pass.vert](/resources/shaders/deferred/lighting_pass.vert) | GLSL | 8 | 0 | 3 | 11 |
| [resources/shaders/forward/blinn\_phong.frag](/resources/shaders/forward/blinn_phong.frag) | GLSL | 156 | 26 | 50 | 232 |
| [resources/shaders/forward/blinn\_phong.vert](/resources/shaders/forward/blinn_phong.vert) | GLSL | 23 | 0 | 6 | 29 |
| [resources/shaders/postprocess/bloom\_blur.frag](/resources/shaders/postprocess/bloom_blur.frag) | GLSL | 28 | 0 | 2 | 30 |
| [resources/shaders/postprocess/bloom\_prefilter.frag](/resources/shaders/postprocess/bloom_prefilter.frag) | GLSL | 14 | 0 | 2 | 16 |
| [resources/shaders/postprocess/post.frag](/resources/shaders/postprocess/post.frag) | GLSL | 26 | 4 | 7 | 37 |
| [resources/shaders/postprocess/quad.vert](/resources/shaders/postprocess/quad.vert) | GLSL | 9 | 0 | 3 | 12 |
| [resources/shaders/postprocess/ssao.frag](/resources/shaders/postprocess/ssao.frag) | GLSL | 34 | 0 | 7 | 41 |
| [resources/shaders/utility/light.frag](/resources/shaders/utility/light.frag) | GLSL | 7 | 0 | 2 | 9 |
| [resources/shaders/utility/light.vert](/resources/shaders/utility/light.vert) | GLSL | 9 | 0 | 2 | 11 |
| [src/core/Application.cpp](/src/core/Application.cpp) | C++ | 138 | 11 | 21 | 170 |
| [src/core/Camera.cpp](/src/core/Camera.cpp) | C++ | 81 | 1 | 14 | 96 |
| [src/core/Framebuffer.cpp](/src/core/Framebuffer.cpp) | C++ | 152 | 3 | 34 | 189 |
| [src/core/Geometry.cpp](/src/core/Geometry.cpp) | C++ | 519 | 85 | 134 | 738 |
| [src/core/Light.cpp](/src/core/Light.cpp) | C++ | 55 | 3 | 7 | 65 |
| [src/core/Material.cpp](/src/core/Material.cpp) | C++ | 87 | 0 | 13 | 100 |
| [src/core/Mesh.cpp](/src/core/Mesh.cpp) | C++ | 49 | 7 | 16 | 72 |
| [src/core/Model.cpp](/src/core/Model.cpp) | C++ | 223 | 17 | 36 | 276 |
| [src/core/Renderer.cpp](/src/core/Renderer.cpp) | C++ | 625 | 55 | 100 | 780 |
| [src/core/Shader.cpp](/src/core/Shader.cpp) | C++ | 118 | 3 | 26 | 147 |
| [src/core/Texture.cpp](/src/core/Texture.cpp) | C++ | 92 | 2 | 14 | 108 |
| [src/main.cpp](/src/main.cpp) | C++ | 8 | 1 | 1 | 10 |
| [src/ui/EditorUI.cpp](/src/ui/EditorUI.cpp) | C++ | 680 | 43 | 60 | 783 |
| [src/utils/FileDialog.cpp](/src/utils/FileDialog.cpp) | C++ | 84 | 6 | 15 | 105 |
| [src/utils/FileSystem.cpp](/src/utils/FileSystem.cpp) | C++ | 23 | 2 | 5 | 30 |
| [src/utils/Logger.cpp](/src/utils/Logger.cpp) | C++ | 37 | 1 | 6 | 44 |
| [xmake.lua](/xmake.lua) | XMake | 18 | 70 | 3 | 91 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)