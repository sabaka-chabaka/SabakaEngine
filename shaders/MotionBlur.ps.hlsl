Texture2D    screenTex : register(t0);
Texture2D    depthTex  : register(t1);
SamplerState sampler0  : register(s0);

cbuffer MotionBlurBuffer : register(b0)
{
    matrix prevViewProj;
    matrix invViewProj;
    float  strength;
    int    numSamples;
    int    enabled;
    float  _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 color = screenTex.Sample(sampler0, input.uv).rgb;

    if (!enabled)
        return float4(color, 1.0);

    float depth = depthTex.Sample(sampler0, input.uv).r;

    if (depth >= 1.0)
        return float4(color, 1.0);

    float2 ndc     = float2(input.uv.x * 2.0 - 1.0, 1.0 - input.uv.y * 2.0);
    float4 clipPos = float4(ndc, depth, 1.0);

    float4 worldPos = mul(invViewProj, clipPos);
    worldPos       /= worldPos.w;

    float4 prevClip = mul(prevViewProj, worldPos);
    float2 prevNDC  = prevClip.xy / prevClip.w;

    float2 velocity = (ndc - prevNDC) * 0.5 * strength;

    float velocityLen = length(velocity);
    if (velocityLen < 0.0001)
        return float4(color, 1.0);

    float3 accumulated = float3(0.0, 0.0, 0.0);
    int    n           = clamp(numSamples, 2, 16);

    for (int i = 0; i < n; ++i)
    {
        float  t      = float(i) / float(n - 1) - 0.5;
        float2 offset = velocity * t;
        float2 sampleUV = saturate(input.uv + offset);
        accumulated    += screenTex.Sample(sampler0, sampleUV).rgb;
    }

    accumulated /= float(n);

    return float4(accumulated, 1.0);
}