#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Floor.h"

Floor::Floor(void)
{
}

Floor::~Floor(void)
{
}

void Floor::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::QUORIDOR_DESK));
	InitModel(DEFAULT_POSITION, DEFAULT_SCALE, DEFAULT_ROTATION);
	transform_.Update();

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("Desk_VS.cso", 0, "Desk_PS.cso", 1);

	// テクスチャのセット
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::MINISHOGI_TEXTURE_TATAMI).handleId_);

	mMaterial_->SetConstBufPS(0, FLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f });

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);
}

void Floor::Update(void)
{
	transform_.Update();
}

void Floor::Draw(void)
{
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
	SetMaxAnisotropy(16);
	mRenderer_->Draw();
}

void Floor::SetPosition(const VECTOR& pos)
{
	transform_.pos = pos;
}