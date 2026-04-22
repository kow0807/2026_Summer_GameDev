#pragma once
#include <DxLib.h>
#include <vector>
#include "../Common/Vector2.h"
#include "../Renderer/ModelMaterial.h"
class Transform;

class ModelRenderer
{

public:

	// 頂点シェーダ用オリジナル定数バッファの使用開始スロット
	static constexpr int CONSTANT_BUF_SLOT_BEGIN_VS = 7;

	// ピクセルシェーダ用オリジナル定数バッファの使用開始スロット
	static constexpr int CONSTANT_BUF_SLOT_BEGIN_PS = 4;

	// コンストラクタ
	ModelRenderer(int modelId_, ModelMaterial& modelMaterial);

	// 配列用
	ModelRenderer(std::vector<Transform>& trans, ModelMaterial& modelMaterial);

	// デストラクタ
	~ModelRenderer(void);

	// 描画
	void Draw(void);

	void DrawModels(int num);

private:

	// モデルのハンドルID
	int modelId_;

	std::vector<Transform> trans_;

	// モデルマテリアル
	ModelMaterial& modelMaterial_;

	// シェーダ設定(頂点)
	void SetReserveVS(void);

	// シェーダ設定(ピクセル)
	void SetReservePS(void);

};
