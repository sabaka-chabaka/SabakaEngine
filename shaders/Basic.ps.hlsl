Texture2D    diffuseMap  : register(t0);
Texture2D    specularMap : register(t1);
SamplerState sampler0    : register(s0);

cbuffer MaterialBuffer : register(b1)
{
    float  specularIntensity;
    float  specularPower;
    float2 uvScale;
    float2 uvOffset;
    float2 _pad;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float3 color       : COLOR;
    float2 uv          : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
    float3 worldPos    : TEXCOORD2;
};

cbuffer LightBuffer : register(b2)
{
    float4 ambientColor;
    float3 lightDirection;
    float  _lightPad;
    float4 lightColor;
    float3 viewPos;
    float  _lightPad2;

    float4 pointLightPos;
    float4 pointLightColor;

    float  attConstant;
    float  attLinear;
    float  attQuadratic;
    float  pointEnabled;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 finalUV   = input.uv * uvScale + uvOffset;
    float4 diffuse   = diffuseMap.Sample(sampler0, finalUV);
    float4 specular  = specularMap.Sample(sampler0, finalUV);

    float3 baseColor = diffuse.rgb * input.color;
    float3 normal    = normalize(input.worldNormal);
    float3 viewDir   = normalize(viewPos - input.worldPos);

    float3 ambient   = baseColor * ambientColor.rgb;

    float3 dirLightDir   = normalize(-lightDirection);
    float  dirDiff       = max(dot(normal, dirLightDir), 0.0);
    float3 dirHalfway    = normalize(dirLightDir + viewDir);
    float  dirSpec       = pow(max(dot(normal, dirHalfway), 0.0), specularPower);
    float3 dirDiffuse    = dirDiff * lightColor.rgb * baseColor;
    float3 dirSpecular   = specular.rgb * specularIntensity * dirSpec * lightColor.rgb;

    float3 ptVec     = pointLightPos.xyz - input.worldPos;
    float  ptDist    = length(ptVec);
    float3 ptDir     = normalize(ptVec);

    float  att       = 1.0 / (attConstant + attLinear * ptDist + attQuadratic * ptDist * ptDist);
    float  ptDiff    = max(dot(normal, ptDir), 0.0);
    float3 ptHalfway = normalize(ptDir + viewDir);
    float  ptSpec    = pow(max(dot(normal, ptHalfway), 0.0), specularPower);

    float3 ptDiffuse  = ptDiff * att * pointLightColor.rgb * baseColor;
    float3 ptSpecular = specular.rgb * specularIntensity * ptSpec * att * pointLightColor.rgb;
    float3 ptContrib  = (ptDiffuse + ptSpecular) * pointEnabled;

    float3 result = saturate(ambient + dirDiffuse + dirSpecular + ptContrib);

    return float4(result, diffuse.a);
}
