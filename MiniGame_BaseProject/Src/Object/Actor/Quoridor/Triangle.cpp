#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Triangle.h"

Triangle::Triangle(void)
{
	r_ = 1.0f;
	g_ = 1.0f;
	b_ = 1.0f;
}

Triangle::~Triangle(void)
{
}

void Triangle::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::QUORIDOR_TRIANGLE));
	InitModel({ 0.0f,10.0f,0.0f }, DEFAULT_SCALE, { 0.0f,0.0f,0.0f });

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("WoodBoard_VS.cso", 0, "WoodBoard_PS.cso", 1);

	//// テクスチャのセット
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::QUORIDOR_TEXTURE_WHITE).handleId_);

	mMaterial_->AddConstBufPS(FLOAT4{ r_, g_, b_, 1.0f });

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);
}

void Triangle::Update(void)
{
	transform_.Update();
}

void Triangle::Draw(void)
{
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);

	SetMaxAnisotropy(16);
	mRenderer_->Draw();
}

void Triangle::SetPositon(float x, float z)
{
	transform_.pos = { x, transform_.pos.y, z };
	transform_.Update();
}

void Triangle::SetRotation(VECTOR rot)
{
	transform_.quaRot = Quaternion::Euler(rot.x, rot.y, rot.z);
	transform_.Update();
}

void Triangle::SetColor(float r, float g, float b)
{
	r_ = r;
	g_ = g;
	b_ = b;
	if (mMaterial_)
	{
		mMaterial_->AddConstBufPS(FLOAT4{ r_, g_, b_, 1.0f });
	}
}