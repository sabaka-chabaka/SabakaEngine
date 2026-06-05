#define NOMINMAX
#include "editor/GizmoRenderer.h"
#include "renderer/GraphicsDevice.h"
#include "renderer/Camera.h"
#include "core/Logger.h"
#include <d3dcompiler.h>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace engine::editor {

    static const char* kGizmoShaderSrc = R"HLSL(
        cbuffer Transform : register(b0) {
            float4x4 mvp;
        };

        struct VSIn  { float3 pos : POSITION; float3 col : COLOR; };
        struct VSOut { float4 pos : SV_POSITION; float3 col : COLOR; };

        VSOut VS(VSIn v) {
            VSOut o;
            o.pos = mul(float4(v.pos, 1.0), mvp);
            o.col = v.col;
            return o;
        }

        float4 PS(VSOut v) : SV_Target { return float4(v.col, 1.0); }
    )HLSL";

    static constexpr float kAxisLen    = 1.0f;
    static constexpr float kStemRadius = 0.025f;
    static constexpr float kConeHeight = 0.22f;
    static constexpr float kConeRadius = 0.065f;
    static constexpr float kBoxSize    = 0.12f;
    static constexpr float kCircleR    = 1.0f;
    static constexpr int   kCircleSeg  = 48;
    static constexpr int   kCylSeg     = 10;

    static XMFLOAT3 bright(const XMFLOAT3& c) {
        return { std::min(c.x + 0.35f, 1.f),
                 std::min(c.y + 0.35f, 1.f),
                 std::min(c.z + 0.35f, 1.f) };
    }

    static const XMFLOAT3 kColX   = { 0.95f, 0.25f, 0.25f };
    static const XMFLOAT3 kColY   = { 0.25f, 0.90f, 0.25f };
    static const XMFLOAT3 kColZ   = { 0.25f, 0.45f, 0.95f };
    static const XMFLOAT3 kColXYZ = { 0.90f, 0.85f, 0.25f };

    void GizmoRenderer::buildArrowMesh(std::vector<GizmoVertex>& verts,
                                       std::vector<uint32_t>& inds,
                                       const XMFLOAT3& dir,
                                       const XMFLOAT3& col,
                                       uint32_t base)
    {
        XMVECTOR d   = XMVector3Normalize(XMLoadFloat3(&dir));
        XMVECTOR up  = XMVectorSet(0,1,0,0);
        XMVECTOR rgt = XMVector3Normalize(
            XMVector3Cross(XMVectorAbs(XMVector3Dot(d, up)) > XMVectorReplicate(0.99f)
                ? XMVectorSet(1,0,0,0) : up, d));
        XMVECTOR fwd = XMVector3Cross(d, rgt);

        float stemEnd = kAxisLen - kConeHeight;

        for (int i = 0; i < kCylSeg; ++i) {
            float a0 = XM_2PI * i       / kCylSeg;
            float a1 = XM_2PI * (i + 1) / kCylSeg;
            auto circle = [&](float a) {
                XMFLOAT3 p;
                XMStoreFloat3(&p, XMVectorAdd(
                    XMVectorScale(rgt, cosf(a) * kStemRadius),
                    XMVectorScale(fwd, sinf(a) * kStemRadius)));
                return p;
            };
            XMFLOAT3 b0 = circle(a0), b1 = circle(a1);
            XMFLOAT3 t0 = { b0.x + dir.x * stemEnd, b0.y + dir.y * stemEnd, b0.z + dir.z * stemEnd };
            XMFLOAT3 t1 = { b1.x + dir.x * stemEnd, b1.y + dir.y * stemEnd, b1.z + dir.z * stemEnd };

            uint32_t v = base + (uint32_t)verts.size();
            verts.push_back({ {0,0,0}, col });
            verts.push_back({ b0,     col });
            verts.push_back({ b1,     col });
            verts.push_back({ t0,     col });
            verts.push_back({ t1,     col });
            inds.insert(inds.end(), { v,v+1,v+2, v+1,v+4,v+2, v+1,v+3,v+4 });
        }

        XMFLOAT3 tipBase = { dir.x * stemEnd, dir.y * stemEnd, dir.z * stemEnd };
        XMFLOAT3 tipTop  = { dir.x * kAxisLen, dir.y * kAxisLen, dir.z * kAxisLen };

        for (int i = 0; i < kCylSeg; ++i) {
            float a0 = XM_2PI * i       / kCylSeg;
            float a1 = XM_2PI * (i + 1) / kCylSeg;
            auto cone = [&](float a) {
                XMFLOAT3 p;
                XMStoreFloat3(&p, XMVectorAdd(
                    XMLoadFloat3(&tipBase),
                    XMVectorAdd(
                        XMVectorScale(rgt, cosf(a) * kConeRadius),
                        XMVectorScale(fwd, sinf(a) * kConeRadius))));
                return p;
            };
            uint32_t v = base + (uint32_t)verts.size();
            verts.push_back({ cone(a0), col });
            verts.push_back({ cone(a1), col });
            verts.push_back({ tipTop,   col });
            inds.insert(inds.end(), { v, v+1, v+2 });
        }
    }

    void GizmoRenderer::buildCircleMesh(std::vector<GizmoVertex>& verts,
                                        std::vector<uint32_t>& inds,
                                        const XMFLOAT3& normal,
                                        const XMFLOAT3& col,
                                        uint32_t base,
                                        int segments)
    {
        XMVECTOR n   = XMVector3Normalize(XMLoadFloat3(&normal));
        XMVECTOR up  = XMVectorSet(0,1,0,0);
        XMVECTOR rgt = XMVector3Normalize(
            XMVector3Cross(XMVectorAbs(XMVector3Dot(n, up)) > XMVectorReplicate(0.99f)
                ? XMVectorSet(1,0,0,0) : up, n));
        XMVECTOR fwd = XMVector3Cross(n, rgt);

        float half = kStemRadius * 1.5f;

        for (int i = 0; i < segments; ++i) {
            float a0 = XM_2PI * i       / segments;
            float a1 = XM_2PI * (i + 1) / segments;
            auto pt = [&](float a, float r) {
                XMFLOAT3 p;
                XMStoreFloat3(&p, XMVectorAdd(
                    XMVectorScale(rgt, cosf(a) * r),
                    XMVectorScale(fwd, sinf(a) * r)));
                return p;
            };
            uint32_t v = base + (uint32_t)verts.size();
            verts.push_back({ pt(a0, kCircleR - half), col });
            verts.push_back({ pt(a0, kCircleR + half), col });
            verts.push_back({ pt(a1, kCircleR + half), col });
            verts.push_back({ pt(a1, kCircleR - half), col });
            inds.insert(inds.end(), { v,v+1,v+2, v,v+2,v+3 });
        }
    }

    void GizmoRenderer::buildBoxHandle(std::vector<GizmoVertex>& verts,
                                       std::vector<uint32_t>& inds,
                                       const XMFLOAT3& dir,
                                       const XMFLOAT3& col,
                                       uint32_t base)
    {
        XMVECTOR d   = XMVector3Normalize(XMLoadFloat3(&dir));
        XMVECTOR up  = XMVectorSet(0,1,0,0);
        XMVECTOR rgt = XMVector3Normalize(
            XMVector3Cross(XMVectorAbs(XMVector3Dot(d, up)) > XMVectorReplicate(0.99f)
                ? XMVectorSet(1,0,0,0) : up, d));
        XMVECTOR fwd = XMVector3Cross(d, rgt);

        XMVECTOR center = XMVectorScale(d, kAxisLen);

        float h = kBoxSize * 0.5f;
        float stemEnd = kAxisLen - h;

        for (int i = 0; i < kCylSeg; ++i) {
            float a0 = XM_2PI * i       / kCylSeg;
            float a1 = XM_2PI * (i + 1) / kCylSeg;
            auto circle = [&](float a) {
                XMFLOAT3 p;
                XMStoreFloat3(&p, XMVectorAdd(
                    XMVectorScale(rgt, cosf(a) * kStemRadius),
                    XMVectorScale(fwd, sinf(a) * kStemRadius)));
                return p;
            };
            XMFLOAT3 b0 = circle(a0), b1 = circle(a1);
            XMFLOAT3 t0 = { b0.x + dir.x * stemEnd, b0.y + dir.y * stemEnd, b0.z + dir.z * stemEnd };
            XMFLOAT3 t1 = { b1.x + dir.x * stemEnd, b1.y + dir.y * stemEnd, b1.z + dir.z * stemEnd };
            uint32_t v = base + (uint32_t)verts.size();
            verts.push_back({ {0,0,0}, col });
            verts.push_back({ b0, col });
            verts.push_back({ b1, col });
            verts.push_back({ t0, col });
            verts.push_back({ t1, col });
            inds.insert(inds.end(), { v,v+1,v+2, v+1,v+4,v+2, v+1,v+3,v+4 });
        }

        XMFLOAT3 corners[8];
        for (int i = 0; i < 8; ++i) {
            float sx = (i & 1) ? h : -h;
            float sy = (i & 2) ? h : -h;
            float sz = (i & 4) ? h : -h;
            XMFLOAT3 c;
            XMStoreFloat3(&c, XMVectorAdd(center,
                XMVectorAdd(XMVectorAdd(
                    XMVectorScale(rgt, sx),
                    XMVectorScale(fwd, sy)),
                    XMVectorScale(d,   sz))));
            corners[i] = c;
        }

        uint32_t faces[6][4] = {
            {0,1,3,2}, {4,6,7,5}, {0,4,5,1},
            {2,3,7,6}, {0,2,6,4}, {1,5,7,3}
        };
        for (auto& f : faces) {
            uint32_t v = base + (uint32_t)verts.size();
            for (int j : f) verts.push_back({ corners[j], col });
            inds.insert(inds.end(), { v,v+1,v+2, v,v+2,v+3 });
        }
    }

    bool GizmoRenderer::compileShaders(ID3D11Device* device) {
        ComPtr<ID3DBlob> vsBlob, psBlob, err;

        HRESULT hr = D3DCompile(kGizmoShaderSrc, strlen(kGizmoShaderSrc),
            nullptr, nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &err);
        if (FAILED(hr)) {
            LOG_ERROR("[Gizmo] VS compile error: " +
                std::string(err ? (char*)err->GetBufferPointer() : "unknown"));
            return false;
        }

        hr = D3DCompile(kGizmoShaderSrc, strlen(kGizmoShaderSrc),
            nullptr, nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &err);
        if (FAILED(hr)) {
            LOG_ERROR("[Gizmo] PS compile error: " +
                std::string(err ? (char*)err->GetBufferPointer() : "unknown"));
            return false;
        }

        device->CreateVertexShader(vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(), nullptr, &m_vs);
        device->CreatePixelShader(psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(), nullptr, &m_ps);

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        device->CreateInputLayout(layout, 2,
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout);

        D3D11_BUFFER_DESC cbd = {};
        cbd.Usage          = D3D11_USAGE_DYNAMIC;
        cbd.ByteWidth      = sizeof(XMMATRIX);
        cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device->CreateBuffer(&cbd, nullptr, &m_cbTransform);

        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&rd, &m_rasterizer);

        D3D11_DEPTH_STENCIL_DESC dd = {};
        dd.DepthEnable    = TRUE;
        dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dd.DepthFunc      = D3D11_COMPARISON_ALWAYS;
        device->CreateDepthStencilState(&dd, &m_depthState);

        return true;
    }

    void GizmoRenderer::init(renderer::GraphicsDevice* device) {
        if (!compileShaders(device->getDevice()))
            return;
        m_initialized = true;
    }

    void GizmoRenderer::setTransformCB(renderer::GraphicsDevice* device,
                                        renderer::Camera* camera,
                                        const XMFLOAT3& origin,
                                        float scale)
    {
        XMMATRIX world = XMMatrixScaling(scale, scale, scale) *
                         XMMatrixTranslation(origin.x, origin.y, origin.z);
        XMMATRIX vp    = camera->getViewMatrix() * camera->getProjectionMatrix();
        XMMATRIX mvp   = XMMatrixTranspose(world * vp);

        auto* ctx = device->getDeviceContext();
        D3D11_MAPPED_SUBRESOURCE ms;
        ctx->Map(m_cbTransform.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, &mvp, sizeof(XMMATRIX));
        ctx->Unmap(m_cbTransform.Get(), 0);
        ctx->VSSetConstantBuffers(0, 1, m_cbTransform.GetAddressOf());
    }

    void GizmoRenderer::uploadAndDraw(renderer::GraphicsDevice* device,
                                       const std::vector<GizmoVertex>& verts,
                                       const std::vector<uint32_t>& inds)
    {
        if (verts.empty() || inds.empty()) return;

        auto* dev = device->getDevice();
        auto* ctx = device->getDeviceContext();

        ComPtr<ID3D11Buffer> vb, ib;

        D3D11_BUFFER_DESC bd = {};
        D3D11_SUBRESOURCE_DATA sd = {};

        bd.Usage          = D3D11_USAGE_DEFAULT;
        bd.ByteWidth      = (UINT)(verts.size() * sizeof(GizmoVertex));
        bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        sd.pSysMem        = verts.data();
        dev->CreateBuffer(&bd, &sd, &vb);

        bd.ByteWidth  = (UINT)(inds.size() * sizeof(uint32_t));
        bd.BindFlags  = D3D11_BIND_INDEX_BUFFER;
        sd.pSysMem    = inds.data();
        dev->CreateBuffer(&bd, &sd, &ib);

        UINT stride = sizeof(GizmoVertex), offset = 0;
        ctx->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);
        ctx->IASetIndexBuffer(ib.Get(), DXGI_FORMAT_R32_UINT, 0);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->DrawIndexed((UINT)inds.size(), 0, 0);
    }

    static XMFLOAT3 axisHighlight(const XMFLOAT3& base, GizmoAxis active, GizmoAxis match) {
        return (active == match) ? bright(base) : base;
    }

    void GizmoRenderer::render(renderer::GraphicsDevice* device,
                                renderer::Camera* camera,
                                const XMFLOAT3& origin,
                                GizmoMode mode,
                                GizmoAxis hovered)
    {
        if (!m_initialized || mode == GizmoMode::None) return;

        XMVECTOR camPos  = XMLoadFloat3(&camera->getPosition());
        XMVECTOR gizPos  = XMLoadFloat3(&origin);
        float    dist    = XMVectorGetX(XMVector3Length(XMVectorSubtract(camPos, gizPos)));
        float    scale   = dist * 0.12f;

        auto* ctx = device->getDeviceContext();
        ctx->VSSetShader(m_vs.Get(), nullptr, 0);
        ctx->PSSetShader(m_ps.Get(), nullptr, 0);
        ctx->IASetInputLayout(m_layout.Get());
        ctx->RSSetState(m_rasterizer.Get());
        ctx->OMSetDepthStencilState(m_depthState.Get(), 0);

        setTransformCB(device, camera, origin, scale);

        std::vector<GizmoVertex> verts;
        std::vector<uint32_t>    inds;
        verts.reserve(512);
        inds.reserve(1024);

        if (mode == GizmoMode::Translate) {
            buildArrowMesh(verts, inds, {1,0,0}, axisHighlight(kColX, hovered, GizmoAxis::X), 0);
            buildArrowMesh(verts, inds, {0,1,0}, axisHighlight(kColY, hovered, GizmoAxis::Y), 0);
            buildArrowMesh(verts, inds, {0,0,1}, axisHighlight(kColZ, hovered, GizmoAxis::Z), 0);

            float q = 0.25f;
            XMFLOAT3 cXY = axisHighlight(kColXYZ, hovered, GizmoAxis::XY);
            XMFLOAT3 cXZ = axisHighlight(kColXYZ, hovered, GizmoAxis::XZ);
            XMFLOAT3 cYZ = axisHighlight(kColXYZ, hovered, GizmoAxis::YZ);
            auto quad = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d, XMFLOAT3 col) {
                uint32_t v = (uint32_t)verts.size();
                verts.push_back({a,col}); verts.push_back({b,col});
                verts.push_back({c,col}); verts.push_back({d,col});
                inds.insert(inds.end(), {v,v+1,v+2, v,v+2,v+3});
            };
            quad({0,0,0},{q,0,0},{q,q,0},{0,q,0}, cXY);
            quad({0,0,0},{q,0,0},{q,0,q},{0,0,q}, cXZ);
            quad({0,0,0},{0,q,0},{0,q,q},{0,0,q}, cYZ);
        }
        else if (mode == GizmoMode::Rotate) {
            buildCircleMesh(verts, inds, {1,0,0}, axisHighlight(kColX, hovered, GizmoAxis::X), 0);
            buildCircleMesh(verts, inds, {0,1,0}, axisHighlight(kColY, hovered, GizmoAxis::Y), 0);
            buildCircleMesh(verts, inds, {0,0,1}, axisHighlight(kColZ, hovered, GizmoAxis::Z), 0);
        }
        else if (mode == GizmoMode::Scale) {
            buildBoxHandle(verts, inds, {1,0,0}, axisHighlight(kColX, hovered, GizmoAxis::X), 0);
            buildBoxHandle(verts, inds, {0,1,0}, axisHighlight(kColY, hovered, GizmoAxis::Y), 0);
            buildBoxHandle(verts, inds, {0,0,1}, axisHighlight(kColZ, hovered, GizmoAxis::Z), 0);

            if (hovered == GizmoAxis::XYZ) {
                uint32_t v = (uint32_t)verts.size();
                float h = kBoxSize * 0.6f;
                XMFLOAT3 corners[8];
                for (int i = 0; i < 8; ++i)
                    corners[i] = { (i&1)?h:-h, (i&2)?h:-h, (i&4)?h:-h };
                for (auto& c : corners) verts.push_back({c, bright(kColXYZ)});
                uint32_t f[6][4] = {{0,1,3,2},{4,6,7,5},{0,4,5,1},{2,3,7,6},{0,2,6,4},{1,5,7,3}};
                for (auto& face : f)
                    inds.insert(inds.end(), {v+face[0],v+face[1],v+face[2],v+face[0],v+face[2],v+face[3]});
            }
        }

        uploadAndDraw(device, verts, inds);

        ctx->OMSetDepthStencilState(nullptr, 0);
    }

}
