Texture2D    screenTex : register(t0);
SamplerState sampler0  : register(s0);

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    float3 hdr = screenTex.Sample(sampler0, uv).rgb;
    float3 ldr = hdr / (hdr + 1.0);
    return float4(ldr, 1.0);
}
