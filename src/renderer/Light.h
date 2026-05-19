#pragma once
#include <DirectXMath.h>
#include <array>

namespace engine::renderer {

    enum class LightType : int {
        Directional = 0,
        Point       = 1,
        Spot        = 2,
    };

    struct alignas(16) GpuLight {
        DirectX::XMFLOAT4 positionAndType    = { 0.0f, 0.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT4 directionAndRange  = { 0.0f, -1.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT4 color              = { 1.0f, 1.0f, 1.0f, 1.0f };

        DirectX::XMFLOAT4 params             = { 1.0f, 0.22f, 0.20f, 1.0f };

        DirectX::XMFLOAT4 spotAngles         = { 0.9848f, 0.9397f, 0.0f, 0.0f };
    };

    static_assert(sizeof(GpuLight) % 16 == 0, "GpuLight must be 16-byte aligned");
    static_assert(sizeof(GpuLight) == 80,      "GpuLight must be exactly 80 bytes");

    static constexpr int MAX_LIGHTS = 16;

    struct alignas(16) LightBuffer {
        DirectX::XMFLOAT4 ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };

        DirectX::XMFLOAT3 viewPos      = { 0.0f, 0.0f, 0.0f };
        int               numLights    = 0;

        std::array<GpuLight, MAX_LIGHTS> lights = {};
    };

    static_assert(sizeof(LightBuffer) % 16 == 0, "LightBuffer must be 16-byte aligned");

    inline GpuLight makeDirectionalLight(
        DirectX::XMFLOAT3 direction,
        DirectX::XMFLOAT3 color,
        float             intensity = 1.0f,
        bool              enabled   = true)
    {
        GpuLight l;
        l.positionAndType   = { 0.0f, 0.0f, 0.0f,
                                static_cast<float>(LightType::Directional) };
        l.directionAndRange = { direction.x, direction.y, direction.z, 0.0f };
        l.color             = { color.x, color.y, color.z, intensity };
        l.params            = { 1.0f, 0.0f, 0.0f, enabled ? 1.0f : 0.0f };
        return l;
    }

    inline GpuLight makePointLight(
        DirectX::XMFLOAT3 position,
        DirectX::XMFLOAT3 color,
        float             intensity    = 1.0f,
        float             range        = 10.0f,
        float             attConstant  = 1.0f,
        float             attLinear    = 0.22f,
        float             attQuadratic = 0.20f,
        bool              enabled      = true)
    {
        GpuLight l;
        l.positionAndType   = { position.x, position.y, position.z,
                                static_cast<float>(LightType::Point) };
        l.directionAndRange = { 0.0f, 0.0f, 0.0f, range };
        l.color             = { color.x, color.y, color.z, intensity };
        l.params            = { attConstant, attLinear, attQuadratic,
                                enabled ? 1.0f : 0.0f };
        return l;
    }

    inline GpuLight makeSpotLight(
        DirectX::XMFLOAT3 position,
        DirectX::XMFLOAT3 direction,
        DirectX::XMFLOAT3 color,
        float             innerDeg     = 10.0f,
        float             outerDeg     = 22.0f,
        float             intensity    = 1.0f,
        float             range        = 15.0f,
        float             attConstant  = 1.0f,
        float             attLinear    = 0.09f,
        float             attQuadratic = 0.032f,
        bool              enabled      = true)
    {
        using namespace DirectX;
        GpuLight l;
        l.positionAndType   = { position.x, position.y, position.z,
                                static_cast<float>(LightType::Spot) };
        l.directionAndRange = { direction.x, direction.y, direction.z, range };
        l.color             = { color.x, color.y, color.z, intensity };
        l.params            = { attConstant, attLinear, attQuadratic,
                                enabled ? 1.0f : 0.0f };
        l.spotAngles        = { cosf(XMConvertToRadians(innerDeg)),
                                cosf(XMConvertToRadians(outerDeg)),
                                0.0f, 0.0f };
        return l;
    }

}