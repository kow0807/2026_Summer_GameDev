#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Shogiban.h"

Shogiban::Shogiban(void)
{
}

Shogiban::~Shogiban(void)
{
}

void Shogiban::Init(void)
{
    transform_.SetModel(
        resMng_.LoadModelDuplicate(
            ResourceManager::SRC::MINISHOGI_SHOGIBAN
        )
    );

    transform_.pos = VGet(0.0f, -195.0f, 0.0f);
    transform_.scl = DEFAULT_SCALE;

    transform_.Update();

    mMaterial_ = std::make_unique<ModelMaterial>(
        "Shogiban_VS.cso",
        0,
        "Shogiban_PS.cso",
        1
    );

    mMaterial_->SetTextureBuf(0, 
        { resMng_.Load(ResourceManager::SRC::MINISHOGI_TEXTURE_WOOD).handleId_});
    mMaterial_->AddConstBufPS(FLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f });

    mRenderer_ = std::make_unique<ModelRenderer>(
        transform_.modelId,
        *mMaterial_
    );
}

void Shogiban::Update(void)
{

    transform_.Update();
}

void Shogiban::Draw(void)
{
    SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
    SetMaxAnisotropy(16);
    SetDrawMode(DX_DRAWMODE_BILINEAR);
    mRenderer_->Draw();
    SetDrawMode(DX_DRAWMODE_NUM);
}
