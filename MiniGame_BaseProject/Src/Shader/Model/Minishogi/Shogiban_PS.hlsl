// VS/PS共通
#include "../../../Shader/Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#define BUMPMAP 1
#include "../../../Shader/Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color; // 色の定数バッファ  
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    float4 texColor =
        diffuseMapTexture.Sample(
            diffuseMapSampler,
            PSInput.uv);

    float3 N = normalize(PSInput.normal);

    // 真上からのメインライト
    float3 mainL =
        normalize(float3(0.0f, 1.0f, 0.0f));

    // 弱い斜め補助光
    float3 fillL =
        normalize(float3(-0.5f, 0.4f, -0.4f));

    float mainDiffuse =
        saturate(dot(N, mainL));

    float fillDiffuse =
        saturate(dot(N, fillL));

    float lighting =
        0.20f +
        mainDiffuse * 0.65f +
        fillDiffuse * 0.15f;

    float3 baseColor = texColor.rgb;

    // 木目のコントラストを少し強める
    baseColor =
        saturate((baseColor - 0.5f) * 1.1f + 0.5f);

    float3 color =
        baseColor *
        g_color.rgb *
        lighting;

    return float4(color, texColor.a);
}