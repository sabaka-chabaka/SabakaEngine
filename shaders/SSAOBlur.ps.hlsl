Texture2D    ssaoTex  : register(t0);
Texture2D    depthTex : register(t1);
SamplerState sampler0 : register(s0);

cbuffer BlurBuffer : register(b0)
{
    float2 texelSize;
    float  depthThreshold;
    float  _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float  centerDepth = depthTex.Sample(sampler0, input.uv).r;
    float  result      = 0.0;
    float  totalWeight = 0.0;

    [unroll]
    for (int x = -2; x <= 2; ++x)
    {
        [unroll]
        for (int y = -2; y <= 2; ++y)
        {
            float2 offset     = float2(x, y) * texelSize;
            float  sampleAO   = ssaoTex.Sample(sampler0, input.uv + offset).r;
            float  sampleDepth = depthTex.Sample(sampler0, input.uv + offset).r;

            float weight = abs(sampleDepth - centerDepth) < depthThreshold ? 1.0 : 0.0;
            result      += sampleAO * weight;
            totalWeight += weight;
        }
    }

    float ao = totalWeight > 0.0 ? result / totalWeight : ssaoTex.Sample(sampler0, input.uv).r;
    return float4(ao, ao, ao, 1.0);
}
