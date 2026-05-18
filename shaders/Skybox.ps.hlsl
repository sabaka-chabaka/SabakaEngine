TextureCube  skyboxMap : register(t0);
SamplerState sampler0  : register(s0);

cbuffer SkyboxBuffer : register(b0)
{
    matrix invViewProj;
    matrix dummy;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 worldPos = mul(float4(input.uv, 1.0f, 1.0f), invViewProj);
    float3 dir = normalize(worldPos.xyz / worldPos.w);

    return skyboxMap.Sample(sampler0, dir);
}
