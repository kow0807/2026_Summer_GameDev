#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Komadai.h"

Komadai::Komadai(void)
{
}

Komadai::~Komadai(void)
{
}

void Komadai::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_KOMADAI));
	InitModel(DEFAULT_POSITION, DEFAULT_SCALE, DEFAULT_ROTATION);
	transform_.Update();

	mMaterial_ = std::make_unique<ModelMaterial>(
		"Shogiban_VS.cso",
		0,
		"Shogiban_VS.cso",
		1
	);

	mMaterial_->SetTextureBuf(0,
		{ resMng_.Load(ResourceManager::SRC::MINISHOGI_TEXTURE_UV).handleId_ });
	mMaterial_->AddConstBufPS(FLOAT4{ 0.147f, 0.104f, 0.161f, 1.0f });

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);
}

void Komadai::Update(void)
{
	transform_.Update();
}

void Komadai::Draw(void)
{
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
	SetMaxAnisotropy(16);
	mRenderer_->Draw();

	SetDrawMode(DX_DRAWMODE_BILINEAR);
	MV1DrawModel(transform_.modelId);
	SetDrawMode(DX_DRAWMODE_NUM);
}

void Komadai::SetPosition(const VECTOR& pos)
{
	transform_.pos = pos;
}