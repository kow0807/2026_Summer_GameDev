#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Grid.h"

Grid::Grid(void)
{
}

Grid::~Grid(void)
{
}

void Grid::Init(void)
{
    transform_.SetModel(
        resMng_.LoadModelDuplicate(
			ResourceManager::SRC::QUORIDOR_GRID
        )
    );

    mMaterial_ = std::make_unique<ModelMaterial>(
        "Grid_VS.cso",
        0,
        "Grid_PS.cso",
        1
    );

    // テクスチャのセット
    mMaterial_->SetTextureBuf(0, resMng_.Load(ResourceManager::SRC::QUORIDOR_TEXTURE_WHITE).handleId_);

    mMaterial_->AddConstBufPS(FLOAT4{ 0.147f, 0.104f, 0.161f, 1.0f });

    mRenderer_ = std::make_unique<ModelRenderer>(
        transform_.modelId,
        *mMaterial_
    );

    transform_.scl = DEFAULT_SCALE;
}

void Grid::Update(void)
{
	transform_.Update();
}

void Grid::Draw(void)
{
	mRenderer_->Draw();
}

void Grid::SetGridSize(const float size)
{
    gridSize_ = size;
}

void Grid::SetBoardPosition(int x, int y)
{
    x_ = x;
    y_ = y;
    UpdateTransform();
}

void Grid::UpdateTransform(void)
{
    transform_.pos = VGet(
        x_ * gridSize_,
        0.0f,
        y_ * gridSize_
    );

    MV1SetPosition(transform_.modelId, transform_.pos);

    transform_.Update();
}