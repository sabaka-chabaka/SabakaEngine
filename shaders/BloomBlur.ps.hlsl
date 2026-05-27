Texture2D    bloomTex : register(t0);
SamplerState sampler0 : register(s0);

cbuffer BlurBuffer : register(b0)
{
    float2 texelSize;
    int    horizontal;
    float  _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

static const float WEIGHTS[9] = {
    0.0093, 0.028, 0.065, 0.121, 0.159, 0.121, 0.065, 0.028, 0.0093
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 dir = horizontal ? float2(texelSize.x, 0.0) : float2(0.0, texelSize.y);

    float3 result = float3(0.0, 0.0, 0.0);
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        float2 offset = dir * float(i - 4);
        result += bloomTex.Sample(sampler0, input.uv + offset).rgb * WEIGHTS[i];
    }

    return float4(result, 1.0);
}
