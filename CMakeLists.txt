cmake_minimum_required(VERSION 3.5)
project(vkyx)

# ADD GLFW 3
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(libs/glfw)

# ADD VULKAN SDK
message(STATUS "Attempting auto locate Vulkan SDK using CMake...")
find_package(Vulkan)
if (NOT Vulkan_FOUND)
    message(STATUS "Failed to locate Vulkan SDK, set VULKAN_SDK Env Variable!")
    set(VULKAN_SDK "/home/loulfy/.wine/drive_c/VulkanSDK")
    set(VULKAN_VERSION "1.0.46.0")
    set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
    set(Vulkan_INCLUDE_DIR "${VULKAN_PATH}/Include")
    set(Vulkan_LIBRARY "${VULKAN_PATH}/Lib/vulkan-1.lib")
    message(STATUS "Using manual specified path: ${VULKAN_PATH}")
endif()

# C++14
set(CMAKE_CXX_STANDARD 14)

# HEADERS
include_directories(libs/glfw/include
                    libs/plog/include
                    ${Vulkan_INCLUDE_DIR})

##########################################

# BUILD 1-hello
add_subdirectory(1-hello)