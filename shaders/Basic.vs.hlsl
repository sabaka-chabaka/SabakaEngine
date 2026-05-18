cbuffer TransformBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix normalMatrix;
};

struct VSInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL;
};

struct VSOutput
{
    float4 position    : SV_POSITION;
    float3 color       : COLOR;
    float2 uv          : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
    float3 worldPos    : TEXCOORD2;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPosition = mul(float4(input.position, 1.0f), model);

    output.position    = mul(mul(worldPosition, view), projection);
    output.color       = input.color;
    output.uv          = input.uv;
    output.worldPos    = worldPosition.xyz;

    output.worldNormal = normalize(mul(input.normal, (float3x3)normalMatrix));

    return output;
}
