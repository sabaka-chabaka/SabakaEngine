cbuffer SkyboxBuffer : register(b0)
{
    matrix viewNoTranslation;
    matrix projection;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position  : SV_POSITION;
    float3 direction : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 pos = float4(input.position, 1.0f);
    pos        = mul(pos, viewNoTranslation);
    pos        = mul(pos, projection);

    output.position  = pos.xyww;
    output.direction = input.position;

    return output;
}
