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
    float3 tangent  : TANGENT;
};

struct VSOutput
{
    float4 position   : SV_POSITION;
    float3 color      : COLOR;
    float2 uv         : TEXCOORD0;
    float3 worldPos   : TEXCOORD1;
    float3 T          : TEXCOORD2;
    float3 B          : TEXCOORD3;
    float3 N          : TEXCOORD4;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPosition = mul(float4(input.position, 1.0f), model);

    output.position = mul(mul(worldPosition, view), projection);
    output.color    = input.color;
    output.uv       = input.uv;
    output.worldPos = worldPosition.xyz;

    float3x3 nm = (float3x3)normalMatrix;
    float3 N = normalize(mul(input.normal,  nm));
    float3 T = normalize(mul(input.tangent, nm));

    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);

    output.T = T;
    output.B = B;
    output.N = N;

    return output;
}
