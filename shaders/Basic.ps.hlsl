Texture2D    diffuseMap  : register(t0);
Texture2D    specularMap : register(t1);
SamplerState sampler0    : register(s0);

cbuffer MaterialBuffer : register(b1)
{
    float  specularIntensity;
    float  specularPower;
    float2 uvScale;
    float2 uvOffset;
    float2 _pad;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float3 color       : COLOR;
    float2 uv          : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
    float3 worldPos    : TEXCOORD2;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 finalUV      = input.uv * uvScale + uvOffset;

    float4 diffuse      = diffuseMap.Sample(sampler0, finalUV);
    float4 specular     = specularMap.Sample(sampler0, finalUV);

    float3 baseColor    = diffuse.rgb * input.color;
    float3 specularGlow = specular.rgb * specularIntensity;

    float3 result       = saturate(baseColor + specularGlow);

    return float4(result, diffuse.a);
}
