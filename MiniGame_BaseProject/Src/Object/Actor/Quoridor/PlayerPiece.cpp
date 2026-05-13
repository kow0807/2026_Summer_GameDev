#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "PlayerPiece.h"

PlayerPiece::PlayerPiece(void)
{
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
        0
    );

    mRenderer_ = std::make_unique<ModelRenderer>(
        transform_.modelId,
        *mMaterial_
    );

    transform_.scl = VGet(0.35f, 0.35f, 0.35f);
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
	mMaterial_->SetConstBufPS(0, FLOAT4{ r, g, b, 1.0f });
}

void PlayerPiece::UpdateTransform(void)
{
    transform_.pos = VGet(
        x_ * GRID_SIZE,
        8.0f,
        y_ * GRID_SIZE
    );

    MV1SetPosition(transform_.modelId, transform_.pos);

    transform_.Update();
}