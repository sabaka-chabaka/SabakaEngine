cbuffer SkyboxBuffer : register(b0)
{
    matrix invViewProj;
    matrix dummy;
};

struct VSOutput
{
    float4 position  : SV_POSITION;
    float2 uv        : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    float2 ndcPos;
    ndcPos.x = (float)(vertexID == 1 ? 3 : -1);
    ndcPos.y = (float)(vertexID == 2 ? 3 : -1);

    VSOutput output;
    output.position = float4(ndcPos, 1.0f, 1.0f);
    output.uv = ndcPos;
    return output;
}
