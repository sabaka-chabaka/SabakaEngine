Texture2D    sceneTex : register(t0);
Texture2D    depthTex : register(t1);
SamplerState sampler0 : register(s0);

cbuffer DofBuffer : register(b0)
{
    float focusDistance;
    float focusRange;
    float maxBlurRadius;
    int   enabled;
    float nearZ;
    float farZ;
    int   numSamples;
    float _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

static const float PI = 3.14159265;

static const float2 DISK_OFFSETS[16] =
{
    float2( 0.000000,  1.000000),
    float2( 0.707107,  0.707107),
    float2( 1.000000,  0.000000),
    float2( 0.707107, -0.707107),
    float2( 0.000000, -1.000000),
    float2(-0.707107, -0.707107),
    float2(-1.000000,  0.000000),
    float2(-0.707107,  0.707107),
    float2( 0.000000,  0.500000),
    float2( 0.500000,  0.000000),
    float2( 0.000000, -0.500000),
    float2(-0.500000,  0.000000),
    float2( 0.353553,  0.353553),
    float2( 0.353553, -0.353553),
    float2(-0.353553, -0.353553),
    float2(-0.353553,  0.353553),
};

float linearizeDepth(float d)
{
    return nearZ * farZ / (farZ - d * (farZ - nearZ));
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 center = sceneTex.Sample(sampler0, input.uv).rgb;

    if (!enabled)
        return float4(center, 1.0);

    float  rawDepth    = depthTex.Sample(sampler0, input.uv).r;

    if (rawDepth >= 1.0)
        return float4(center, 1.0);

    float  linearD     = linearizeDepth(rawDepth);
    float  coc         = saturate(abs(linearD - focusDistance) / max(focusRange, 0.001));
    float  blurRadius  = coc * maxBlurRadius;

    if (blurRadius < 0.0005)
        return float4(center, 1.0);

    float2 texelSize = float2(ddx(input.uv.x), ddy(input.uv.y));
    texelSize        = abs(float2(1.0 / (1.0 / texelSize.x), 1.0 / (1.0 / texelSize.y)));

    uint   w, h;
    sceneTex.GetDimensions(w, h);
    float2 invRes = float2(1.0 / float(w), 1.0 / float(h));

    int    n           = clamp(numSamples, 4, 16);
    float3 accumulated = float3(0.0, 0.0, 0.0);
    float  weight      = 0.0;

    for (int i = 0; i < n; ++i)
    {
        float2 offset    = DISK_OFFSETS[i] * blurRadius * invRes;
        float2 sampleUV  = saturate(input.uv + offset);
        float  sDepth    = depthTex.Sample(sampler0, sampleUV).r;
        float  sLinear   = linearizeDepth(sDepth);
        float  sCoc      = saturate(abs(sLinear - focusDistance) / max(focusRange, 0.001));
        float  w0        = max(sCoc, coc);
        accumulated     += sceneTex.Sample(sampler0, sampleUV).rgb * w0;
        weight          += w0;
    }

    accumulated /= max(weight, 0.0001);

    float  blend  = smoothstep(0.0, 1.0, coc);
    float3 result = lerp(center, accumulated, blend);

    return float4(result, 1.0);
}
