Texture2D    depthTex   : register(t0);
Texture2D    normalsTex : register(t1);
Texture2D    noiseTex   : register(t2);
SamplerState sampler0   : register(s0);
SamplerState samplerWrap : register(s1);

cbuffer SSAOBuffer : register(b0)
{
    float4   samples[64];
    matrix   projection;
    matrix   invProjection;
    float2   noiseScale;
    float    radius;
    float    bias;
    int      numSamples;
    float3   _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float3 reconstructViewPos(float2 uv, float depth, matrix invProj)
{
    float4 ndc = float4(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0, depth, 1.0);
    float4 vs  = mul(invProj, ndc);
    return vs.xyz / vs.w;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float  depth     = depthTex.Sample(sampler0, input.uv).r;

    if (depth >= 1.0)
        return float4(1.0, 1.0, 1.0, 1.0);

    float3 fragPos   = reconstructViewPos(input.uv, depth, invProjection);
    float3 normal    = normalize(normalsTex.Sample(sampler0, input.uv).xyz * 2.0 - 1.0);
    float3 randomVec = normalize(noiseTex.Sample(samplerWrap, input.uv * noiseScale).xyz * 2.0 - 1.0);

    float3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN     = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        float3 sampleVS  = mul(samples[i].xyz, TBN);
        sampleVS         = fragPos + sampleVS * radius;

        float4 projected = mul(projection, float4(sampleVS, 1.0));
        float2 sampleUV  = projected.xy / projected.w * 0.5 + 0.5;
        sampleUV.y       = 1.0 - sampleUV.y;

        float sampleDepth = depthTex.Sample(sampler0, sampleUV).r;
        float3 samplePos  = reconstructViewPos(sampleUV, sampleDepth, invProjection);

        float rangeCheck  = smoothstep(0.0, 1.0, radius / abs(fragPos.z - samplePos.z));
        occlusion        += (samplePos.z >= sampleVS.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    float ao = 1.0 - (occlusion / float(numSamples));
    return float4(ao, ao, ao, 1.0);
}
