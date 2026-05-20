#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "PlayerPiece.h"

PlayerPiece::PlayerPiece(void)
{
	r_ = 1.0f;
	g_ = 1.0f;
	b_ = 1.0f;

    transform_.scl = DEFAULT_SCALE;
}

PlayerPiece::~PlayerPiece(void)
{
}

void PlayerPiece::Init(void)
{
    transform_.SetModel(
        resMng_.LoadModelDuplicate(
            ResourceManager::SRC::QUORIDOR_PIECE
        )
    );

    mMaterial_ = std::make_unique<ModelMaterial>(
        "Piece_VS.cso",
        0,
        "Piece_PS.cso",
        1
    );

    mMaterial_->AddConstBufPS( FLOAT4{ r_, g_, b_,1.0f });


    mRenderer_ = std::make_unique<ModelRenderer>(
        transform_.modelId,
        *mMaterial_
    );

}

void PlayerPiece::Update(void)
{
    transform_.Update();
}

void PlayerPiece::Draw(void)
{
    SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
    SetMaxAnisotropy(16);
	mRenderer_->Draw();
}

void PlayerPiece::SetBoardPosition(int x, int y)
{
    x_ = x;
    y_ = y;
    UpdateTransform();
}

void PlayerPiece::SetColor(float r, float g, float b)
{
	r_ = r;
	g_ = g;
	b_ = b;

	mMaterial_->SetConstBufPS(0, FLOAT4{ r_, g_, b_, 1.0f });
}

void PlayerPiece::SetCellSize(float cellSize)
{
	cellSize_ = cellSize;
}

void PlayerPiece::UpdateTransform(void)
{
    transform_.pos = VGet(
        x_ * cellSize_,
        8.0f,
        y_ * cellSize_
    );

    MV1SetPosition(transform_.modelId, transform_.pos);


    transform_.Update();
}