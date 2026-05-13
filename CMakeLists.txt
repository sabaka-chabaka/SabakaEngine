cmake_minimum_required(VERSION 3.28)
project(SabakaEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(SabakaEngine WIN32 src/main.cpp
        src/platform/Window.cpp
        src/platform/Window.h
        src/renderer/GraphicsDevice.cpp
        src/renderer/GraphicsDevice.h
        src/core/Application.cpp
        src/core/Application.h
)

target_include_directories(SabakaEngine PRIVATE src)

target_link_libraries(SabakaEngine PRIVATE
        d3d11
        dxgi
        d3dcompiler
)

install(TARGETS SabakaEngine DESTINATION .)