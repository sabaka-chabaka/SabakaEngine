cmake_minimum_required(VERSION 3.28)
project(SabakaEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(SabakaEngine src/main.cpp
        src/platform/Window.cpp
        src/platform/Window.h
)

target_include_directories(SabakaEngine PRIVATE src)

target_link_libraries(SabakaEngine PRIVATE
        d3d11
        dxgi
        d3dcompiler
)

install(TARGETS SabakaEngine DESTINATION .)