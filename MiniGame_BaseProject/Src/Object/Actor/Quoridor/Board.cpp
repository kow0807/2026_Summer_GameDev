#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Board.h"

Board::Board(void)
{
}

Board::~Board(void)
{
}

void Board::Init(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::BOARD));
	InitModel(DEFAULT_POSITION, DEFAULT_SCALE, DEFAULT_ROTATION);
	transform_.Update();

	// モデル描画用の初期化
	mMaterial_ = std::make_unique<ModelMaterial>("WoodBoard_VS.cso", 0, "WoodBoard_PS.cso", 0);
	mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::WOOD_BOARD_TEXTURE).handleId_);

	mRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *mMaterial_);

	//// デバッグ用
	//printfDx("VS: %d\n", mMaterial_->GetShaderVS());
	//printfDx("PS: %d\n", mMaterial_->GetShaderPS());
}

void Board::Update(void)
{
	transform_.Update();
}

void Board::Draw(void)
{
	mRenderer_->Draw();
}

void Board::SetPosition(const VECTOR& pos)
{
	transform_.pos = pos;
}