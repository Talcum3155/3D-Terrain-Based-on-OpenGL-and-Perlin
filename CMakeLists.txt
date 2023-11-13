cmake_minimum_required(VERSION 3.26)
project(3DPerlinMap)

set(CMAKE_CXX_STANDARD 20)

# Specify the location of the "include" folder.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

# Specify the location of the "lib" folder.
# when linking, it will search for the DLL at this location
link_directories(${PROJECT_SOURCE_DIR}/lib)

file(GLOB_RECURSE SOURCES "*.cpp" "*.h" "*.hpp" "*.c")

# 过滤掉cmake-build-debug文件夹下的文件
list(FILTER SOURCES EXCLUDE REGEX "cmake-build-debug/.*")

add_executable(3DPerlinMap ${SOURCES})

# Specify dll location
target_link_libraries(3DPerlinMap ${PROJECT_SOURCE_DIR}/lib/glfw3.dll)