#pragma once
#include <string>

#include "renderer/Mesh.h"

namespace engine::io {
    class SmeshLoader {
    public:
        static renderer::Mesh load(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const std::string &path);

        static bool isValid(const std::string& path);
    };
}