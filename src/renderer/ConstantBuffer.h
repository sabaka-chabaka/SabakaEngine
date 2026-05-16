#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <stdexcept>
 
namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    template<typename T>
    class ConstantBuffer {
    public:
        ConstantBuffer(ID3D11Device* device, ID3D11DeviceContext* context)
            : m_context(context)
        {
            static_assert((sizeof(T) % 16) == 0,
                "ConstantBuffer size must be a multiple of 16 bytes");

            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth         = sizeof(T);
            desc.Usage             = D3D11_USAGE_DYNAMIC;
            desc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE;

            HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffer);
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create constant buffer");
            }
        }

        void update(const T& data) {
            D3D11_MAPPED_SUBRESOURCE mapped = {};

            HRESULT hr = m_context->Map(
                m_buffer.Get(),
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped
            );

            if (FAILED(hr)) {
                throw std::runtime_error("Failed to map constant buffer");
            }

            memcpy(mapped.pData, &data, sizeof(T));
            m_context->Unmap(m_buffer.Get(), 0);
        }

        void bindVS(unsigned int slot) const {
            m_context->VSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
        }

        void bindPS(unsigned int slot) const {
            m_context->PSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
        }

    private:
        ID3D11DeviceContext*  m_context;
        ComPtr<ID3D11Buffer>  m_buffer;
    };

}