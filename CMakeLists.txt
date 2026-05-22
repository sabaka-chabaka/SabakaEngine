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
        src/core/Logger.cpp
        src/core/Logger.h
        src/renderer/Mesh.cpp
        src/renderer/Mesh.h
        src/renderer/Shader.cpp
        src/renderer/Shader.h
        src/renderer/Camera.cpp
        src/renderer/Camera.h
        src/renderer/ConstantBuffer.h
        src/renderer/TransformData.h
        src/platform/Input.cpp
        src/platform/Input.h
        src/renderer/Texture2D.cpp
        src/renderer/Texture2D.h
        src/renderer/SamplerState.cpp
        src/renderer/SamplerState.h
        src/renderer/Material.h
        src/renderer/CubemapTexture.cpp
        src/renderer/CubemapTexture.h
        src/renderer/SkyboxMesh.cpp
        src/renderer/SkyboxMesh.h
        src/renderer/SkyboxBuffer.h
        src/renderer/Light.h
        src/core/Transform.cpp
        src/core/Transform.h
        src/core/Component.h
        src/core/Entity.cpp
        src/core/Entity.h
        src/core/Scene.cpp
        src/core/Scene.h
        src/core/SceneNode.cpp
        src/core/SceneNode.h
        src/core/SceneHierarchy.cpp
        src/core/SceneHierarchy.h
        src/renderer/Material.cpp
        src/core/MeshRenderer.cpp
        src/core/MeshRenderer.h
        src/io/ObjLoader.cpp
        src/io/ObjLoader.h
        src/math/AABB.cpp
        src/math/AABB.h
)

target_include_directories(SabakaEngine PRIVATE src)

target_link_libraries(SabakaEngine PRIVATE
        d3d11
        dxgi
        d3dcompiler
)

add_custom_command(TARGET SabakaEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:SabakaEngine>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/textures"
        "$<TARGET_FILE_DIR:SabakaEngine>/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models"
        "$<TARGET_FILE_DIR:SabakaEngine>/models"
        COMMENT "Copying shaders, models and textures to output directory"
)

install(TARGETS SabakaEngine DESTINATION .)