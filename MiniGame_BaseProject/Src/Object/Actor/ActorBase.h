#pragma once
#include <memory>
#include "../Common/Transform.h"
class ResourceManager;
class SceneManager;
class ModelMaterial;
class ModelRenderer;

class ActorBase
{

public:

	// コンストラクタ
	ActorBase(void);

	// デストラクタ
	virtual ~ActorBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;

	const Transform& GetTransform(void) const;

protected:

	// シングルトン参照
	ResourceManager& resMng_;
	SceneManager& scnMng_;

	// モデル制御の基本情報
	Transform transform_;

	// モデル描画用
	std::unique_ptr<ModelMaterial> mMaterial_;
	std::unique_ptr<ModelRenderer> mRenderer_;

	// モデル初期化
	void InitModel(VECTOR pos, VECTOR scl, VECTOR quaRotLocal);

};
