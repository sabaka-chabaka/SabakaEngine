Texture2D    screenTex : register(t0);
SamplerState sampler0  : register(s0);

cbuffer FXAABuffer : register(b0)
{
    float2 texelSize;
    int    enabled;
    float  _pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

static const float FXAA_EDGE_THRESHOLD     = 0.125;
static const float FXAA_EDGE_THRESHOLD_MIN = 0.0625;
static const float FXAA_SUBPIX_QUALITY     = 0.75;
static const int   FXAA_SEARCH_STEPS       = 12;
static const float FXAA_SEARCH_THRESHOLD   = 0.25;

static const float QUALITY[12] = { 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0, 8.0 };

float luma(float3 rgb)
{
    return dot(rgb, float3(0.299, 0.587, 0.114));
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 rgbM = screenTex.Sample(sampler0, input.uv).rgb;

    if (!enabled)
        return float4(rgbM, 1.0);

    float lumaM = luma(rgbM);
    float lumaN = luma(screenTex.Sample(sampler0, input.uv + float2( 0.0,  1.0) * texelSize).rgb);
    float lumaS = luma(screenTex.Sample(sampler0, input.uv + float2( 0.0, -1.0) * texelSize).rgb);
    float lumaE = luma(screenTex.Sample(sampler0, input.uv + float2( 1.0,  0.0) * texelSize).rgb);
    float lumaW = luma(screenTex.Sample(sampler0, input.uv + float2(-1.0,  0.0) * texelSize).rgb);

    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaE, lumaW)));
    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float localContrast = lumaMax - lumaMin;

    if (localContrast < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD))
        return float4(rgbM, 1.0);

    float lumaNW = luma(screenTex.Sample(sampler0, input.uv + float2(-1.0,  1.0) * texelSize).rgb);
    float lumaNE = luma(screenTex.Sample(sampler0, input.uv + float2( 1.0,  1.0) * texelSize).rgb);
    float lumaSW = luma(screenTex.Sample(sampler0, input.uv + float2(-1.0, -1.0) * texelSize).rgb);
    float lumaSE = luma(screenTex.Sample(sampler0, input.uv + float2( 1.0, -1.0) * texelSize).rgb);

    float edgeH = abs(lumaNW + lumaN + lumaNE - lumaSW - lumaS - lumaSE) * (1.0 / 3.0)
                + abs(lumaW  + lumaM + lumaE  - 0.0   - 0.0   - 0.0   ) * (2.0 / 3.0);

    float edgeV = abs(lumaNW + lumaW + lumaSW - lumaNE - lumaE - lumaSE) * (1.0 / 3.0)
                + abs(lumaN  + lumaM + lumaS  - 0.0   - 0.0   - 0.0   ) * (2.0 / 3.0);

    bool isHorizontal = edgeH >= edgeV;

    float stepLen   = isHorizontal ? texelSize.y : texelSize.x;
    float lumaPos   = isHorizontal ? lumaN        : lumaE;
    float lumaNeg   = isHorizontal ? lumaS        : lumaW;

    float gradPos = abs(lumaPos - lumaM);
    float gradNeg = abs(lumaNeg - lumaM);
    bool  posIsSteeper = gradPos >= gradNeg;

    float gradient  = posIsSteeper ? gradPos   : gradNeg;
    float lumaLocal = posIsSteeper ? (lumaM + lumaPos) * 0.5 : (lumaM + lumaNeg) * 0.5;

    float2 uvStep = isHorizontal ? float2(0.0, stepLen * (posIsSteeper ? 1.0 : -1.0))
                                 : float2(stepLen * (posIsSteeper ? 1.0 : -1.0), 0.0);

    float2 uvP = input.uv + (isHorizontal ? float2( texelSize.x, 0.0) : float2(0.0,  texelSize.y)) + uvStep * 0.5;
    float2 uvN = input.uv + (isHorizontal ? float2(-texelSize.x, 0.0) : float2(0.0, -texelSize.y)) + uvStep * 0.5;

    float2 dirStep = isHorizontal ? float2(texelSize.x, 0.0) : float2(0.0, texelSize.y);

    float lumaEndP = luma(screenTex.Sample(sampler0, uvP).rgb) - lumaLocal;
    float lumaEndN = luma(screenTex.Sample(sampler0, uvN).rgb) - lumaLocal;

    bool doneP = abs(lumaEndP) >= gradient * FXAA_SEARCH_THRESHOLD;
    bool doneN = abs(lumaEndN) >= gradient * FXAA_SEARCH_THRESHOLD;

    [unroll]
    for (int i = 0; i < FXAA_SEARCH_STEPS; ++i)
    {
        if (!doneP)
        {
            uvP      += dirStep * QUALITY[i];
            lumaEndP  = luma(screenTex.Sample(sampler0, uvP).rgb) - lumaLocal;
            doneP     = abs(lumaEndP) >= gradient * FXAA_SEARCH_THRESHOLD;
        }
        if (!doneN)
        {
            uvN      -= dirStep * QUALITY[i];
            lumaEndN  = luma(screenTex.Sample(sampler0, uvN).rgb) - lumaLocal;
            doneN     = abs(lumaEndN) >= gradient * FXAA_SEARCH_THRESHOLD;
        }
        if (doneP && doneN) break;
    }

    float distP = isHorizontal ? abs(uvP.x - input.uv.x) : abs(uvP.y - input.uv.y);
    float distN = isHorizontal ? abs(uvN.x - input.uv.x) : abs(uvN.y - input.uv.y);

    bool  nearerIsP    = distP < distN;
    float nearerDist   = min(distP, distN);
    float edgeLen      = distP + distN;
    float pixelOffset  = 0.5 - nearerDist / edgeLen;

    float lumaNearer   = nearerIsP ? lumaEndP : lumaEndN;
    bool  goodSpan     = ((lumaM - lumaLocal) < 0.0) != (lumaNearer < 0.0);
    float edgeOffset   = goodSpan ? pixelOffset : 0.0;

    float lumaAvg     = (lumaN + lumaS + lumaE + lumaW + lumaNW + lumaNE + lumaSW + lumaSE) * (1.0 / 8.0);
    float subpixDelta = abs(lumaAvg - lumaM) / localContrast;
    float subpixBlend = subpixDelta * subpixDelta * FXAA_SUBPIX_QUALITY;

    float finalOffset = max(edgeOffset, subpixBlend);

    float2 finalUV = input.uv + (isHorizontal ? float2(0.0, finalOffset * stepLen)
                                              : float2(finalOffset * stepLen, 0.0));

    float3 result = screenTex.Sample(sampler0, finalUV).rgb;
    return float4(result, 1.0);
}
