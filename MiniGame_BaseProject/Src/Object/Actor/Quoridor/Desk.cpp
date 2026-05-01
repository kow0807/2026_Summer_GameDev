#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Desk.h"

Desk::Desk(void)
{
}

Desk::~Desk(void)
{
}

void Desk::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::BOARD));
	InitModel(DEFAULT_POSITION, DEFAULT_SCALE, DEFAULT_ROTATION);
	transform_.Update();

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("WoodBoard_VS.cso", 0, "WoodBoard_PS.cso", 0);
	
	// テクスチャのセット
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::WOOD_BOARD_TEXTURE).handleId_);
	mMaterial_->SetTextureBuf(1, resMng_.Load(ResourceManager::SRC::WOOD_BOARD_TEXTURE_N).handleId_);

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);

	//// デバッグ用
	//printfDx("VS: %d\n", mMaterial_->GetShaderVS());
	//printfDx("PS: %d\n", mMaterial_->GetShaderPS());
}

void Desk::Update(void)
{
	transform_.Update();
}

void Desk::Draw(void)
{
	//SetDrawMode(DX_DRAWMODE_BILINEAR);
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
	SetMaxAnisotropy(16);
	mRenderer_->Draw();
}

void Desk::SetPosition(const VECTOR& pos)
{
	transform_.pos = pos;
}