// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit


// PS
#define BUMPMAP 1
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color; // 色の定数バッファ  
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{    
    float3 ambientColor = float3(0.5f, 0.5f, 0.5f);
    
    return float4(g_color.rgb, 1.0f);
}