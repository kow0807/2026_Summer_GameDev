// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

//// 定数バッファ：スロット4番目(b4と書く)
//cbuffer cbParam : register(b4)
//{
    
//}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    // 元の色を取得
    float4 defaultColor = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    
    // ライトの方向を定義(これは真上から)
    float3 lightDir = normalize(float3(0.0f, -1.0f, 0.0f));
    
    // 拡散光の計算
    float diffuse = max(dot(PSInput.normal, lightDir), 0.0f);
    
    // 拡散光込みの色を計算
    float3 color = defaultColor * diffuse;
    
    // このままだと真っ暗になる可能性があるので、環境光を加える
    float3 ambientColor = float3(0.3f, 0.3f, 0.3f);
    color += ambientColor * float3(defaultColor.r, defaultColor.g, defaultColor.b);
    
    return float4(color.r, color.g, color.b, 1.0f);
}