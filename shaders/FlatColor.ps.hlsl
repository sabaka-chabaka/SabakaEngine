cbuffer TransformBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix normalMatrix;
};

cbuffer MaterialBuffer : register(b1)
{
    float  specularIntensity;
    float  specularPower;
    float2 uvScale;
    float2 uvOffset;
    float  useNormalMap;
    float  _matPad;
};

cbuffer LightBuffer : register(b2)
{
    float4 ambientColor;
    float3 viewPos;
    int    numLights;
};

cbuffer ShadowBuffer : register(b3)
{
    matrix lightSpaceMatrix;
    float3 lightDir;
    float  shadowBias;
};

struct PSInput
{
    float4 position  : SV_POSITION;
    float3 color     : COLOR;
    float2 uv        : TEXCOORD0;
    float3 worldPos  : TEXCOORD1;
    float3 T         : TEXCOORD2;
    float3 B         : TEXCOORD3;
    float3 N         : TEXCOORD4;
    float4 shadowPos : TEXCOORD5;
};

struct PSOutput
{
    float4 color  : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

PSOutput main(PSInput input)
{
    float3 N       = normalize(input.N);
    float3 L       = normalize(float3(1, -1, 1));
    float  diff    = saturate(dot(N, -L)) * 0.6 + 0.4;
    float3 baseCol = float3(0.6, 0.6, 0.65);
    float3 result  = baseCol * ambientColor.rgb + baseCol * diff;

    float3 viewN = normalize(mul(float4(N, 0.0), view).xyz);
    float3 encN  = viewN * 0.5 + 0.5;

    PSOutput output;
    output.color  = float4(saturate(result), 1.0);
    output.normal = float4(encN, 1.0);
    return output;
}
