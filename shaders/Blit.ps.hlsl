Texture2D    screenTex : register(t0);
SamplerState sampler0  : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 hdr = screenTex.Sample(sampler0, input.uv).rgb;
    float3 ldr = hdr / (hdr + 1.0);
    return float4(ldr, 1.0);
}
