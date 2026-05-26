cbuffer ShadowBuffer : register(b3)
{
    matrix lightSpaceMatrix;
    float3 lightDir;
    float  shadowBias;
};

cbuffer TransformBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix normalMatrix;
};

float4 main(float3 position : POSITION) : SV_POSITION
{
    float4 worldPos = mul(float4(position, 1.0f), model);
    return mul(worldPos, lightSpaceMatrix);
}
