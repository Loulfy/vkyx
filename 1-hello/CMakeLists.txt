cmake_minimum_required(VERSION 3.5)
project(hello)

# FILES
file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

add_executable(hello ${SOURCES} ${HEADERS})
target_link_libraries(hello ${Vulkan_LIBRARY} glfw)