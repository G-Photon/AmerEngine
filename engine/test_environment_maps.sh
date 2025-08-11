#!/bin/bash

# 环境贴图管理系统测试脚本

echo "=== 环境贴图管理系统功能测试 ==="

# 检查必要的资源文件
echo "1. 检查资源文件..."

if [ -f "resources/textures/hdr/newport_loft.hdr" ]; then
    echo "✓ HDR环境贴图文件存在: newport_loft.hdr"
else
    echo "✗ 缺少HDR环境贴图文件: newport_loft.hdr"
fi

if [ -d "resources/textures/skybox" ]; then
    skybox_files=("right.jpg" "left.jpg" "top.jpg" "bottom.jpg" "front.jpg" "back.jpg")
    missing_files=()
    
    for file in "${skybox_files[@]}"; do
        if [ -f "resources/textures/skybox/$file" ]; then
            echo "✓ 天空盒文件存在: $file"
        else
            echo "✗ 缺少天空盒文件: $file"
            missing_files+=("$file")
        fi
    done
    
    if [ ${#missing_files[@]} -eq 0 ]; then
        echo "✓ 所有天空盒文件完整"
    else
        echo "✗ 缺少 ${#missing_files[@]} 个天空盒文件"
    fi
else
    echo "✗ 缺少天空盒目录: resources/textures/skybox"
fi

# 检查IBL着色器
echo -e "\n2. 检查IBL着色器..."

ibl_shaders=(
    "resources/shaders/ibl/cubemap.vert"
    "resources/shaders/ibl/equirectangular_to_cubemap.frag"
    "resources/shaders/ibl/irradiance_convolution.frag"
    "resources/shaders/ibl/prefilter.frag"
    "resources/shaders/ibl/brdf_lut.frag"
)

for shader in "${ibl_shaders[@]}"; do
    if [ -f "$shader" ]; then
        echo "✓ IBL着色器存在: $(basename $shader)"
    else
        echo "✗ 缺少IBL着色器: $(basename $shader)"
    fi
done

# 检查编译状态
echo -e "\n3. 检查编译状态..."
if [ -f "build/windows/x64/debug/AmerEngine.exe" ] || [ -f "build/windows/x64/release/AmerEngine.exe" ]; then
    echo "✓ 项目已编译"
else
    echo "? 项目可能需要重新编译"
    echo "  运行: xmake build"
fi

echo -e "\n=== 功能验证清单 ==="
echo "启动程序后，请验证以下功能："
echo "1. □ 打开渲染器设置面板"
echo "2. □ 展开'光照'→'环境贴图管理'"
echo "3. □ 确认显示当前环境贴图名称"
echo "4. □ 在下拉菜单中切换环境贴图"
echo "5. □ 点击'添加HDR环境贴图'按钮"
echo "6. □ 测试选择新的HDR文件"
echo "7. □ 验证IBL光照实时更新"
echo "8. □ 检查环境贴图详情列表"

echo -e "\n=== 测试HDR文件建议 ==="
echo "可以下载这些免费HDR环境贴图进行测试："
echo "• HDRi Haven: https://hdrihaven.com/"
echo "• Poly Haven: https://polyhaven.com/hdris"
echo "• 建议格式: .hdr (Radiance HDR)"
echo "• 建议分辨率: 1024x512 或更高"

echo -e "\n测试脚本完成！"
