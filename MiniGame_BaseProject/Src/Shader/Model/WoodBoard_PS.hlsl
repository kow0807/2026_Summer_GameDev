// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit


// PS
#define BUMPMAP 1
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

//// 定数バッファ：スロット4番目(b4と書く)
//cbuffer cbParam : register(b4)
//{
    
//}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    float2 uv = PSInput.uv;
    float2 offset = 1.0f / float2(512.0f, 512.0f); // テクスチャのサイズに応じて調整
    
    // 元の色を取得
    float4 defaultColor = 0;
    defaultColor = diffuseMapTexture.Sample(diffuseMapSampler, uv);
    
    
    // ノーマルマップから法線を取得
    float3 normalTexture = normalMapTexture.Sample(normalMapSampler, uv).xyz;
    
    // 法線を[-1, 1]の範囲に変換
    normalTexture = normalTexture * 2.0f - 1.0f;
    
    // DXの法線はY軸が反転しているため、Y成分を反転
    normalTexture.y *= -1;
    
    // 元の法線
    float3 originalNormal = normalize(PSInput.normal);
    
    // ノーマルの影響度を弱める
    float strength = 0.5f; // 0.0fでノーマルマップの影響なし、1.0fで完全にノーマルマップの法線
    normalTexture = normalize(lerp(originalNormal, normalTexture, strength));
    
    // 法線マップと元の法線を組み合わせる
    float3 combinedNormal = normalize(originalNormal + normalTexture);
    
    // ライトの方向を定義(これは真上から)
    float3 lightDir = normalize(float3(0.0f, -1.0f, 0.0f));
    
    // 拡散光の計算
    float diffuse = max(dot(combinedNormal, lightDir), 0.0f);
    
    // 疑似AA
    float edge = fwidth(diffuse);
    diffuse = smoothstep(0.0f, edge, diffuse);
    
    // 拡散光込みの色を計算
    float3 color = defaultColor.rgb * diffuse;
    
    // このままだと真っ暗になる可能性があるので、環境光を加える
    float3 ambientColor = float3(0.3f, 0.3f, 0.3f);
    color += ambientColor * defaultColor.rgb;
    
    return float4(color.rgb, 1.0f);
}