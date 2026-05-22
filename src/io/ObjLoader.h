#pragma once
#include "renderer/Mesh.h"
#include <d3d11.h>
#include <string>

namespace engine::io {
    class ObjLoader {
    public:
        static renderer::Mesh load(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const std::string& path
            );
    };
}