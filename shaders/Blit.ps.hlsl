Texture2D    screenTex : register(t0);
SamplerState sampler0  : register(s0);

cbuffer PostProcessBuffer : register(b0)
{
    float exposure;
    int   tonemapMode;
    float _pad0;
    float _pad1;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float3 tonemapReinhard(float3 color)
{
    return color / (color + 1.0);
}

float3 tonemapACES(float3 color)
{
    color *= 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

float3 gammaCorrect(float3 color)
{
    return pow(max(color, 0.0), 1.0 / 2.2);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 hdr = screenTex.Sample(sampler0, input.uv).rgb;

    hdr *= exposure;

    float3 ldr;
    if (tonemapMode == 1)
        ldr = tonemapACES(hdr);
    else
        ldr = tonemapReinhard(hdr);

    float3 srgb = gammaCorrect(ldr);

    return float4(srgb, 1.0);
}
