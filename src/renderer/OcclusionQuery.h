#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    class OcclusionQuery {
    public:
        explicit OcclusionQuery(ID3D11Device* device);
        ~OcclusionQuery() = default;

        OcclusionQuery(const OcclusionQuery&)            = delete;
        OcclusionQuery& operator=(const OcclusionQuery&) = delete;

        void begin(ID3D11DeviceContext* context);
        void end(ID3D11DeviceContext* context);

        bool isVisible(ID3D11DeviceContext* context) const;

    private:
        ComPtr<ID3D11Query> m_query;
        mutable bool        m_lastResult = true;
    };
}