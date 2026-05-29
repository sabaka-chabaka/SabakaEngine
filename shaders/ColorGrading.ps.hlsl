Texture2D     sceneTex : register(t0);
Texture3D     lutTex   : register(t1);
SamplerState  sampler0 : register(s0);

cbuffer ColorGradingBuffer : register(b0)
{
    float lutStrength;
    int   lutEnabled;
    float _pad0;
    float _pad1;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

static const float LUT_SIZE = 16.0;

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 color = sceneTex.Sample(sampler0, input.uv).rgb;

    if (!lutEnabled)
        return float4(color, 1.0);

    float3 lutCoord = saturate(color) * (LUT_SIZE - 1.0) / LUT_SIZE + 0.5 / LUT_SIZE;
    float3 graded   = lutTex.Sample(sampler0, lutCoord).rgb;

    float3 result = lerp(color, graded, saturate(lutStrength));

    return float4(result, 1.0);
}
