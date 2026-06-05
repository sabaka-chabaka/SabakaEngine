cmake_minimum_required(VERSION 3.28)
project(SabakaEngine)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
    SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/nlohmann
)
FetchContent_MakeAvailable(nlohmann_json)
set(NLOHMANN_JSON_INCLUDE ${CMAKE_BINARY_DIR}/_deps/nlohmann)

if(NOT DEFINED PHYSX_ROOT)
    set(PHYSX_ROOT "D:/CLionProjects/SabakaEngine/third_party/PhysX-sdk")
endif()

if(NOT DEFINED QT_ROOT)
    set(QT_ROOT "D:/Qt/6.11.1/msvc2022_64")
endif()
list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT}")

set(ENGINE_SOURCES
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
        src/core/BoundingBoxComponent.cpp
        src/core/BoundingBoxComponent.h
        src/math/Frustum.cpp
        src/math/Frustum.h
        src/renderer/InstancedMesh.cpp
        src/renderer/InstancedMesh.h
        src/renderer/DepthPrePass.cpp
        src/renderer/DepthPrePass.h
        src/renderer/OcclusionQuery.cpp
        src/renderer/OcclusionQuery.h
        src/core/LodComponent.cpp
        src/core/LodComponent.h
        src/renderer/Shadowmap.cpp
        src/renderer/Shadowmap.h
        src/renderer/ShadowPass.cpp
        src/renderer/ShadowPass.h
        src/renderer/ShadowSampler.cpp
        src/renderer/ShadowSampler.h
        src/renderer/CascadeShadowMap.cpp
        src/renderer/CascadeShadowMap.h
        src/renderer/PostProcessPass.cpp
        src/renderer/PostProcessPass.h
        src/renderer/RenderTarget.cpp
        src/renderer/RenderTarget.h
        src/renderer/PostProcessBuffer.h
        src/renderer/FXAABuffer.h
        src/renderer/BloomBuffer.h
        src/renderer/SSAOBuffer.h
        src/renderer/MotionBlurBuffer.h
        src/renderer/DofBuffer.h
        src/renderer/ColorGradingBuffer.h
        src/assets/AssetId.h
        src/assets/AssetHandle.h
        src/assets/AssetManager.cpp
        src/assets/AssetManager.h
        src/io/SmeshFormat.h
        src/io/SmeshLoader.cpp
        src/io/SmeshLoader.h
        src/io/MeshImporter.cpp
        src/io/MeshImporter.h
        src/core/ThreadPool.h
        src/assets/AsyncAssetHandle.h
        src/core/VFS.h
        src/core/FileWatcher.h
        src/physics/ColliderShape.h
        src/physics/PhysicsWorld.cpp
        src/physics/PhysicsWorld.h
        src/physics/RigidBody.cpp
        src/physics/RigidBody.h
        src/audio/AudioEngine.cpp
        src/audio/AudioEngine.h
        src/audio/AudioClip.cpp
        src/audio/AudioClip.h
        src/audio/AudioSource.cpp
        src/audio/AudioSource.h
        src/audio/AudioMixer.cpp
        src/audio/AudioMixer.h
)

set(ENGINE_LIBS
        d3d11
        dxgi
        d3dcompiler
        xaudio2
        ${PHYSX_ROOT}/lib/PhysX_64.lib
        ${PHYSX_ROOT}/lib/PhysXCommon_64.lib
        ${PHYSX_ROOT}/lib/PhysXFoundation_64.lib
        ${PHYSX_ROOT}/lib/PhysXExtensions_static_64.lib
        ${PHYSX_ROOT}/lib/PhysXPvdSDK_static_64.lib
        ${PHYSX_ROOT}/lib/LowLevel_static_64.lib
        ${PHYSX_ROOT}/lib/LowLevelAABB_static_64.lib
        ${PHYSX_ROOT}/lib/LowLevelDynamics_static_64.lib
        ${PHYSX_ROOT}/lib/SceneQuery_static_64.lib
        ${PHYSX_ROOT}/lib/SimulationController_static_64.lib
        ${PHYSX_ROOT}/lib/PhysXTask_static_64.lib
)

add_library(SabakaEngineLib STATIC ${ENGINE_SOURCES})

target_include_directories(SabakaEngineLib PUBLIC
        src
        ${PHYSX_ROOT}/include
)

target_link_libraries(SabakaEngineLib PUBLIC ${ENGINE_LIBS})

add_executable(SabakaRuntime WIN32
        src/main.cpp
        src/runtime/EngineRuntime.cpp
        src/runtime/EngineRuntime.h
)

target_include_directories(SabakaRuntime PRIVATE src)
target_link_libraries(SabakaRuntime PRIVATE SabakaEngineLib)

add_custom_command(TARGET SabakaRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:SabakaRuntime>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/textures"
        "$<TARGET_FILE_DIR:SabakaRuntime>/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models"
        "$<TARGET_FILE_DIR:SabakaRuntime>/models"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHYSX_ROOT}/bin/PhysX_64.dll"
        "$<TARGET_FILE_DIR:SabakaRuntime>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHYSX_ROOT}/bin/PhysXCommon_64.dll"
        "$<TARGET_FILE_DIR:SabakaRuntime>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHYSX_ROOT}/bin/PhysXFoundation_64.dll"
        "$<TARGET_FILE_DIR:SabakaRuntime>"
        COMMENT "Copying assets and PhysX DLLs to SabakaRuntime output"
)

find_package(Qt6 COMPONENTS Core Widgets QUIET)

if(Qt6_FOUND)
    message(STATUS "Qt6 found — building SabakaEditor")

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    add_executable(SabakaEditor
            src/editor/main.cpp
            src/editor/MainWindow.cpp
            src/editor/MainWindow.h
            src/editor/ViewportWindow.cpp
            src/editor/ViewportWindow.h
            src/editor/EditorApplication.cpp
            src/editor/EditorApplication.h
            src/editor/HierarchyWidget.cpp
            src/editor/HierarchyWidget.h
            src/editor/EditorCameraController.cpp
            src/editor/EditorCameraController.h
            src/editor/InspectorWidget.cpp
            src/editor/InspectorWidget.h
            src/editor/sections/ComponentSection.cpp
            src/editor/sections/ComponentSection.h
            src/editor/sections/TransformSection.cpp
            src/editor/sections/TransformSection.h
            src/editor/sections/MeshRendererSection.cpp
            src/editor/sections/MeshRendererSection.h
            src/editor/sections/BoundingBoxSection.cpp
            src/editor/sections/BoundingBoxSection.h
            src/editor/AssetBrowserWidget.cpp
            src/editor/AssetBrowserWidget.h
            src/editor/ViewportDropHandler.cpp
            src/editor/ViewportDropHandler.h
            src/editor/GizmoMode.h
            src/editor/GizmoRenderer.cpp
            src/editor/GizmoRenderer.h
            src/editor/GizmoController.cpp
            src/editor/GizmoController.h
            src/editor/SceneSerializer.cpp
            src/editor/SceneSerializer.h
            src/editor/PlayModeManager.cpp
            src/editor/PlayModeManager.h
            src/editor/commands/Command.h
            src/editor/commands/CommandHistory.cpp
            src/editor/commands/CommandHistory.h
            src/editor/commands/TransformCommands.cpp
            src/editor/commands/TransformCommands.h
            src/editor/commands/EntityCommands.cpp
            src/editor/commands/EntityCommands.h
    )

    target_include_directories(SabakaEditor PRIVATE src ${NLOHMANN_JSON_INCLUDE})

    target_link_libraries(SabakaEditor PRIVATE
            SabakaEngineLib
            Qt6::Core
            Qt6::Widgets
    )

    add_custom_command(TARGET SabakaEditor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:SabakaEditor>/shaders"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/textures"
            "$<TARGET_FILE_DIR:SabakaEditor>/textures"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/models"
            "$<TARGET_FILE_DIR:SabakaEditor>/models"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PHYSX_ROOT}/bin/PhysX_64.dll"
            "$<TARGET_FILE_DIR:SabakaEditor>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PHYSX_ROOT}/bin/PhysXCommon_64.dll"
            "$<TARGET_FILE_DIR:SabakaEditor>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PHYSX_ROOT}/bin/PhysXFoundation_64.dll"
            "$<TARGET_FILE_DIR:SabakaEditor>"
            COMMAND "${QT_ROOT}/bin/windeployqt.exe"
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            "$<TARGET_FILE:SabakaEditor>"
            COMMENT "Copying assets, PhysX DLLs and Qt runtime to SabakaEditor output"
    )
else()
    message(STATUS "Qt6 not found — SabakaEditor will not be built")
endif()

install(TARGETS SabakaRuntime DESTINATION .)