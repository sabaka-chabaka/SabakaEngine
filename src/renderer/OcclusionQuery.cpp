#include "renderer/OcclusionQuery.h"
#include <stdexcept>

namespace engine::renderer {

    OcclusionQuery::OcclusionQuery(ID3D11Device* device) {
        D3D11_QUERY_DESC desc = {};
        desc.Query = D3D11_QUERY_OCCLUSION;

        if (FAILED(device->CreateQuery(&desc, &m_query)))
            throw std::runtime_error("OcclusionQuery: failed to create query");
    }

    void OcclusionQuery::begin(ID3D11DeviceContext* context) {
        context->Begin(m_query.Get());
    }

    void OcclusionQuery::end(ID3D11DeviceContext* context) {
        context->End(m_query.Get());
    }

    bool OcclusionQuery::isVisible(ID3D11DeviceContext* context) const {
        UINT64 pixelCount = 0;
        HRESULT hr = context->GetData(
            m_query.Get(), &pixelCount, sizeof(pixelCount), D3D11_ASYNC_GETDATA_DONOTFLUSH
        );

        if (hr == S_OK) {
            m_lastResult = pixelCount > 0;
        }

        return m_lastResult;
    }
}