Texture2D    sceneTex : register(t0);
SamplerState sampler0 : register(s0);

cbuffer BloomBuffer : register(b0)
{
    float threshold;
    float intensity;
    float2 _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 color      = sceneTex.Sample(sampler0, input.uv).rgb;
    float  brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    float  contrib    = max(brightness - threshold, 0.0) / max(brightness, 0.0001);
    return float4(color * contrib, 1.0);
}
