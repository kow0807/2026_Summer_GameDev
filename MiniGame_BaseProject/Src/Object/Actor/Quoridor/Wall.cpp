#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Wall.h"

constexpr float CELL_SIZE = 50.0f;

Wall::Wall(void)
{
}

Wall::~Wall(void)
{
}

void Wall::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::WOOD_WALL));

	UpdateTransform();

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("WoodBoard_VS.cso", 0, "WoodBoard_PS.cso", 0);

	// テクスチャのセット
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::WOOD_BOARD_TEXTURE).handleId_);
	mMaterial_->SetTextureBuf(1, resMng_.Load(ResourceManager::SRC::WOOD_BOARD_TEXTURE_N).handleId_);

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);

}

void Wall::Update(void)
{
	transform_.Update();
}

void Wall::Draw(void)
{
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);

	SetMaxAnisotropy(16);
	mRenderer_->Draw();
}

int Wall::GetX(void) const
{
	return x_;
}

int Wall::GetY(void) const
{
	return y_;
}

Wall::TYPE Wall::GetType(void) const
{
	return type_;
}

void Wall::DrawPreview(bool canPlace)
{
	// 半透明
	MV1SetOpacityRate(
		transform_.modelId,
		0.5f
	);

	// 色変更
	if (canPlace)
	{
		MV1SetDifColorScale(
			transform_.modelId,
			GetColorF(0.3f, 1.0f, 0.3f,1.0f)
		);
	}
	else
	{
		MV1SetDifColorScale(
			transform_.modelId,
			GetColorF(1.0f, 0.3f, 0.3f, 1.0f)
		);
	}

	mRenderer_->Draw();

	// 戻す

	MV1SetOpacityRate(
		transform_.modelId,
		1.0f
	);

	MV1SetDifColorScale(
		transform_.modelId,
		GetColorF(1.0f, 1.0f, 1.0f, 1.0f)
	);
}

void Wall::SetBoardPosition(int x, int y)
{
	x_ = x;
	y_ = y;

	UpdateTransform();
}

void Wall::SetType(TYPE type)
{
	type_ = type;
	UpdateTransform();
}

void Wall::RefreshTransform(void)
{
	UpdateTransform();
}

void Wall::UpdateTransform(void)
{
	if (type_ == TYPE::VERTICAL)
	{
		transform_.pos = VGet(
			x_ * CELL_SIZE + CELL_SIZE * 0.5f,
			0.0f,
			y_ * CELL_SIZE + CELL_SIZE
		);

		transform_.quaRotLocal = Quaternion::Euler({ AsoUtility::Deg2RadF(0.0f),
						 AsoUtility::Deg2RadF(0.0f),AsoUtility::Deg2RadF(0.0f) });

	}
	else
	{
		transform_.pos = VGet(
			x_ * CELL_SIZE + CELL_SIZE,
			0.0f,
			y_ * CELL_SIZE + CELL_SIZE * 0.5f
		);

		transform_.quaRotLocal = Quaternion::Euler({ AsoUtility::Deg2RadF(0.0f),
						 AsoUtility::Deg2RadF(90.0f),AsoUtility::Deg2RadF(0.0f) });

	}

	MV1SetPosition(transform_.modelId, transform_.pos);

	transform_.scl = DEFAULT_SCALE;

	transform_.Update();
}
