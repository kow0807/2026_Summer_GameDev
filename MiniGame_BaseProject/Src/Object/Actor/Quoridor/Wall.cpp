#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Wall.h"

Wall::Wall(void)
{
	r_ = 1.0f;
	g_ = 1.0f;
	b_ = 1.0f;
	x_ = 0;
	y_ = 0;
	type_ = TYPE::VERTICAL;
	cellSize_ = 0.0f;
}

Wall::~Wall(void)
{
}

void Wall::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::QUORIDOR_WALL));

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("WoodBoard_VS.cso", 0, "WoodBoard_PS.cso", 1);

	//// テクスチャのセット
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::QUORIDOR_TEXTURE_WHITE).handleId_);

	mMaterial_->AddConstBufPS(FLOAT4{ r_, g_, b_, 1.0f });

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);

}

void Wall::InitTransform(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::QUORIDOR_WALL));

	UpdateTransform();

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("Wall_VS.cso", 0, "Wall_PS.cso", 1);

	// テクスチャのセット
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::QUORIDOR_TEXTURE_WHITE).handleId_);

	mMaterial_->AddConstBufPS(FLOAT4{ r_, g_, b_, 1.0f });

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

void Wall::DrawPreview(bool canPlace,VECTOR wallColor)
{

	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);

	SetMaxAnisotropy(16);


	// 色変更
	if (canPlace)
	{
		/*r_ = 1.0f;
		g_ = 0.70f;
		b_ = 0.0f;*/

		r_ = wallColor.x;
		g_ = wallColor.y;
		b_ = wallColor.z;
	}
	else
	{
		r_ = 1.0f;
		g_ = 0.0f;
		b_ = 0.0f;
	}

	mMaterial_->SetConstBufPS(0, FLOAT4{ r_, g_, b_, 1.0f });

	mRenderer_->Draw();
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

void Wall::SetCellSize(float cellSize)
{
	cellSize_ = cellSize;
	UpdateTransform();
}

void Wall::RefreshTransform(void)
{
	UpdateTransform();
}

void Wall::SetColor(float r, float g, float b)
{
	r_ = r;
	g_ = g;
	b_ = b;

	mMaterial_->SetConstBufPS(0, FLOAT4{ r_, g_, b_, 1.0f });
}


void Wall::UpdateTransform(void)
{
	if (type_ == TYPE::VERTICAL)
	{
		transform_.pos = VGet(
			(x_ + 1) * cellSize_ - cellSize_ * 0.5f,
			0.0f,
			y_ * cellSize_ + cellSize_ * 0.5f
		);

		transform_.quaRotLocal = Quaternion::Euler({ AsoUtility::Deg2RadF(0.0f),
						 AsoUtility::Deg2RadF(0.0f),AsoUtility::Deg2RadF(0.0f) });
	}
	else
	{
		transform_.pos = VGet(
			x_ * cellSize_ + cellSize_ * 0.5f,
			0.0f,
			y_ * cellSize_ + cellSize_ * 0.5f
		);

		transform_.quaRotLocal = Quaternion::Euler({ AsoUtility::Deg2RadF(0.0f),
						 AsoUtility::Deg2RadF(90.0f),AsoUtility::Deg2RadF(0.0f) });

	}

	MV1SetPosition(transform_.modelId, transform_.pos);

	transform_.scl = DEFAULT_SCALE;

	transform_.Update();
}
