Texture2D    diffuseMap   : register(t0);
Texture2D    specularMap  : register(t1);
Texture2D    normalMap    : register(t2);
Texture2D    shadowMap    : register(t3);
SamplerState sampler0     : register(s0);

SamplerComparisonState shadowSampler : register(s1);

cbuffer MaterialBuffer : register(b1)
{
    float  specularIntensity;
    float  specularPower;
    float2 uvScale;
    float2 uvOffset;
    float  useNormalMap;
    float  _matPad;
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

#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT       1
#define LIGHT_SPOT        2
#define MAX_LIGHTS        16

struct GpuLight
{
    float4 positionAndType;
    float4 directionAndRange;
    float4 color;
    float4 params;
    float4 spotAngles;
};

cbuffer LightBuffer : register(b2)
{
    float4   ambientColor;
    float3   viewPos;
    int      numLights;
    GpuLight lights[MAX_LIGHTS];
};

cbuffer ShadowBuffer : register(b3)
{
    matrix lightSpaceMatrix;
    float3 lightDir;
    float  shadowBias;
};

float calcShadowPCF(float4 shadowPos, float3 N)
{
    float3 proj = shadowPos.xyz / shadowPos.w;

    if (proj.x < -1.0 || proj.x > 1.0 ||
        proj.y < -1.0 || proj.y > 1.0 ||
        proj.z <  0.0 || proj.z > 1.0)
        return 1.0;

    float2 uv;
    uv.x =  proj.x * 0.5 + 0.5;
    uv.y = -proj.y * 0.5 + 0.5;

    float depth = proj.z;

    float NdotL = dot(N, normalize(-lightDir));
    float bias  = max(shadowBias * 10.0 * (1.0 - NdotL), shadowBias);

    float2 shadowMapSize;
    shadowMap.GetDimensions(shadowMapSize.x, shadowMapSize.y);
    float2 texelSize = 1.0 / shadowMapSize;

    float shadow = 0.0;
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            shadow += shadowMap.SampleCmpLevelZero(shadowSampler, uv + offset, depth - bias);
        }
    }
    shadow /= 9.0;

    return shadow;
}

float calcAttenuation(GpuLight light, float dist)
{
    float range = light.directionAndRange.w;
    if (range > 0.0 && dist > range) return 0.0;
    float c = light.params.x;
    float l = light.params.y;
    float q = light.params.z;
    return 1.0 / (c + l * dist + q * dist * dist);
}

float3 calcBlinnPhong(
    float3 L, float3 N, float3 V,
    float3 baseColor, float3 specTex,
    float3 lightColor, float intensity)
{
    float  diff    = max(dot(N, L), 0.0);
    float3 halfway = normalize(L + V);
    float  spec    = pow(max(dot(N, halfway), 0.0), specularPower);
    float3 diffuse  = diff  * lightColor * baseColor  * intensity;
    float3 specular = specTex * specularIntensity * spec * lightColor * intensity;
    return diffuse + specular;
}

float3 calcDirectional(GpuLight light, float3 N, float3 V,
                       float3 baseColor, float3 specTex, float shadow)
{
    float3 L = normalize(-light.directionAndRange.xyz);
    return calcBlinnPhong(L, N, V, baseColor, specTex,
                          light.color.rgb, light.color.a) * shadow;
}

float3 calcPoint(GpuLight light, float3 worldPos,
                 float3 N, float3 V, float3 baseColor, float3 specTex)
{
    float3 vec  = light.positionAndType.xyz - worldPos;
    float  dist = length(vec);
    float3 L    = normalize(vec);
    float  att  = calcAttenuation(light, dist);
    return calcBlinnPhong(L, N, V, baseColor, specTex,
                          light.color.rgb, light.color.a) * att;
}

float3 calcSpot(GpuLight light, float3 worldPos,
                float3 N, float3 V, float3 baseColor, float3 specTex)
{
    float3 vec      = light.positionAndType.xyz - worldPos;
    float  dist     = length(vec);
    float3 L        = normalize(vec);
    float  theta    = dot(L, normalize(-light.directionAndRange.xyz));
    float  cosInner = light.spotAngles.x;
    float  cosOuter = light.spotAngles.y;
    float  epsilon  = cosInner - cosOuter;
    float  spotF    = saturate((theta - cosOuter) / epsilon);
    float  att      = calcAttenuation(light, dist) * spotF;
    return calcBlinnPhong(L, N, V, baseColor, specTex,
                          light.color.rgb, light.color.a) * att;
}

float4 main(PSInput input) : SV_TARGET
{
    float2 finalUV  = input.uv * uvScale + uvOffset;
    float4 diffuse  = diffuseMap.Sample(sampler0, finalUV);
    float4 specular = specularMap.Sample(sampler0, finalUV);

    float3 baseColor = diffuse.rgb * input.color;
    float3 V         = normalize(viewPos - input.worldPos);

    float3 N;
    if (useNormalMap > 0.5)
    {
        float3 nTangent = normalMap.Sample(sampler0, finalUV).rgb * 2.0 - 1.0;
        float3x3 TBN = float3x3(
            normalize(input.T),
            normalize(input.B),
            normalize(input.N)
        );
        N = normalize(mul(nTangent, TBN));
    }
    else
    {
        N = normalize(input.N);
    }

    float shadow = calcShadowPCF(input.shadowPos, N);

    float3 result = baseColor * ambientColor.rgb;

    for (int i = 0; i < numLights; ++i)
    {
        if (lights[i].params.w < 0.5) continue;

        int type = (int)lights[i].positionAndType.w;

        if (type == LIGHT_DIRECTIONAL)
            result += calcDirectional(lights[i], N, V, baseColor, specular.rgb, shadow);
        else if (type == LIGHT_POINT)
            result += calcPoint(lights[i], input.worldPos, N, V, baseColor, specular.rgb);
        else if (type == LIGHT_SPOT)
            result += calcSpot(lights[i], input.worldPos, N, V, baseColor, specular.rgb);
    }

    return float4(saturate(result), diffuse.a);
}
