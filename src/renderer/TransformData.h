#pragma once
#include <DirectXMath.h>

namespace engine::renderer {

    struct alignas(16) TransformData {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX normalMatrix;
    };

}