cmake_minimum_required(VERSION 3.20)
project(SabakaEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SabakaEngine WIN32 src/main.cpp
        src/platform/Window.cpp
        src/platform/Window.h
        src/renderer/GraphicsDevice.cpp
        src/renderer/GraphicsDevice.h
        src/core/Application.cpp
        src/core/Application.h
        src/core/Logger.cpp
        src/core/Logger.h
        src/renderer/Mesh.cpp
        src/renderer/Mesh.h
        src/renderer/Shader.cpp
        src/renderer/Shader.h
)

target_include_directories(SabakaEngine PRIVATE src)

target_link_libraries(SabakaEngine PRIVATE
        d3d11
        dxgi
        d3dcompiler
)

add_custom_command(TARGET SabakaEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:SabakaEngine>/shaders
        COMMENT "Copying shaders to output directory"
)