Texture2D    diffuseMap : register(t0);
SamplerState sampler0   : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
    float2 uv       : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = diffuseMap.Sample(sampler0, input.uv);
    return texColor * float4(input.color, 1.0f);
}
