TextureCube  skyboxMap : register(t0);
SamplerState sampler0  : register(s0);

struct PSInput
{
    float4 position  : SV_POSITION;
    float3 direction : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 dir = normalize(input.direction);
    return skyboxMap.Sample(sampler0, dir);
}
