cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_TOOLCHAIN_FILE "c:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_PREFIX_PATH "C:/Graphics/VulkanProjects/Manaburn/vcpkg_installed/x86-windows")
set(ENV{VULKAN_SDK} "c:/VulkanSDK/1.3.261.1")
set(CMAKE_CXX_STANDARD 20)
project(Manaburn VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(app)
add_subdirectory(graphics)


