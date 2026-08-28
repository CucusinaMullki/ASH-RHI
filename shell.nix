{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "vulkan-rhi-dev";

  nativeBuildInputs = with pkgs; [
    cmake
    gnumake
    pkg-config
    gcc
    gdb
    clang-tools
  ];

  buildInputs = with pkgs; [
    vulkan-headers
    vulkan-loader
    vulkan-tools
    vulkan-validation-layers

    shaderc
    spirv-tools

    glfw
  ];

  shellHook = ''
    export VULKAN_SDK="${pkgs.vulkan-headers}"
    export VK_LAYER_PATH="${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d"
    export LD_LIBRARY_PATH="${pkgs.vulkan-loader}/lib:$LD_LIBRARY_PATH"

    echo "Vulkan RHI dev shell готов."
    echo "Проверка: vulkaninfo | head -n 20"
  '';
}
