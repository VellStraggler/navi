cmake_minimum_required(VERSION 3.10)
project(MyGLFWApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW paths
set(GLFW_INCLUDE_DIR "C:/code-tools/glfw/include")
set(GLFW_LIB_DIR "C:/code-tools/glfw/build/src")

# Add your executable (note: main.cpp is inside version0/)
add_executable(MyGLFWApp 
        version0/window.cpp
        version0/processedaudio.cpp
        version0/miniaudio_impl.cpp)

# Include GLFW headers
target_include_directories(MyGLFWApp PRIVATE ${GLFW_INCLUDE_DIR})

# Link with static GLFW and system OpenGL/GDI libraries
target_link_directories(MyGLFWApp PRIVATE ${GLFW_LIB_DIR})
target_link_libraries(MyGLFWApp PRIVATE glfw3 opengl32 gdi32)
