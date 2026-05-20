#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "BoardBase.h"

BoardBase::BoardBase(void)
{
}

BoardBase::~BoardBase(void)
{
}

void BoardBase::Init(void)
{
    transform_.SetModel(
        resMng_.LoadModelDuplicate(
            ResourceManager::SRC::QUORIDOR_BASE
        )
    );

    transform_.pos = VGet(200.0f, -10.0f, 200.0f);
    transform_.scl = DEFAULT_SCALE;

    transform_.Update();

    mMaterial_ = std::make_unique<ModelMaterial>(
        "BoardBase_VS.cso",
        0,
        "BoardBase_PS.cso",
        1
    );

	mMaterial_->AddConstBufPS(FLOAT4{ 0.147f, 0.104f, 0.161f, 1.0f });

    mRenderer_ = std::make_unique<ModelRenderer>(
        transform_.modelId,
        *mMaterial_
    );
}

void BoardBase::Update(void)
{
	transform_.Update();
}

void BoardBase::Draw(void)
{
    SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
    SetMaxAnisotropy(16);
	mRenderer_->Draw();
}
