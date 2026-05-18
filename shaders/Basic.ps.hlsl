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
};

float4 main(PSInput input) : SV_TARGET
{
    float2 finalUV      = input.uv * uvScale + uvOffset;

    float4 diffuse      = diffuseMap.Sample(sampler0, finalUV);
    float4 specular     = specularMap.Sample(sampler0, finalUV);

    float3 baseColor    = diffuse.rgb * input.color;
    float3 ambient      = baseColor * ambientColor.rgb;
    float3 specularGlow = specular.rgb * specularIntensity;

    float3 normal       = normalize(input.worldNormal);
    float3 lightDir     = normalize(-lightDirection);
    float  diff         = max(dot(normal, lightDir), 0.0);
    float3 diffuseLight = diff * lightColor.rgb;

    float3 viewDir      = normalize(viewPos - input.worldPos);
    float3 reflectDir   = reflect(-lightDir, normal);
    float  specPhong    = pow(max(dot(viewDir, reflectDir), 0.0), specularPower);

    float3 halfwayDir   = normalize(lightDir + viewDir);
    float  specBlinn    = pow(max(dot(normal, halfwayDir), 0.0), specularPower);

    float3 specularFinal = specular.rgb * specularIntensity * specBlinn * lightColor.rgb;

    float3 result       = saturate(ambient + (diffuseLight * baseColor) + specularFinal + specularGlow);

    return float4(result, diffuse.a);
}
