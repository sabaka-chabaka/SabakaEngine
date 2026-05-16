cbuffer TransformBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct VSInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 pos = float4(input.position, 1.0f);

    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.position = pos;
    output.color    = input.color;

    return output;
}
