Texture2D    sceneTex : register(t0);
SamplerState sampler0 : register(s0);

cbuffer VignetteBuffer : register(b0)
{
    float innerRadius;
    float outerRadius;
    float intensity;
    int   vignetteEnabled;
    float aberrationStrength;
    int   aberrationEnabled;
    float _pad0;
    float _pad1;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 centered = input.uv - 0.5;
    float  dist     = length(centered);
    float2 dir      = dist > 0.0001 ? normalize(centered) : float2(0.0, 0.0);

    float3 color;

    if (aberrationEnabled)
    {
        float  shift = dist * aberrationStrength;
        float  r = sceneTex.Sample(sampler0, saturate(input.uv + dir * shift)).r;
        float  g = sceneTex.Sample(sampler0, input.uv).g;
        float  b = sceneTex.Sample(sampler0, saturate(input.uv - dir * shift)).b;
        color    = float3(r, g, b);
    }
    else
    {
        color = sceneTex.Sample(sampler0, input.uv).rgb;
    }

    if (vignetteEnabled)
    {
        float vignette = 1.0 - smoothstep(innerRadius, outerRadius, dist);
        vignette       = lerp(1.0, vignette, intensity);
        color         *= vignette;
    }

    return float4(color, 1.0);
}
