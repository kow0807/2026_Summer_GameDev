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
    float2 uv = PSInput.uv;

    float4 defaultColor =
        diffuseMapTexture.Sample(diffuseMapSampler, uv);

    float3 normal =
        normalize(PSInput.normal);

    float3 lightDir =
        normalize(float3(0.3f, 1.0f, 0.5f));

    float diffuse =
        saturate(dot(normal, lightDir));

    diffuse = diffuse * 0.8f + 0.2f;

    float3 color =
        defaultColor.rgb * diffuse;

    return float4(color, 1.0f);
}