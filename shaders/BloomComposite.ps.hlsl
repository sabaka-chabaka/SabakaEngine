Texture2D    sceneTex : register(t0);
Texture2D    bloomTex : register(t1);
Texture2D    ssaoTex  : register(t2);
SamplerState sampler0 : register(s0);

cbuffer BloomBuffer : register(b0)
{
    float threshold;
    float intensity;
    float ssaoEnabled;
    float _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 scene = sceneTex.Sample(sampler0, input.uv).rgb;
    float3 bloom = bloomTex.Sample(sampler0, input.uv).rgb;
    float  ao    = ssaoEnabled > 0.5 ? ssaoTex.Sample(sampler0, input.uv).r : 1.0;

    float3 result = scene * ao + bloom * intensity;
    return float4(result, 1.0);
}
