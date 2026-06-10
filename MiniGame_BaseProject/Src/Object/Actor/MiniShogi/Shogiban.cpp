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

    transform_.pos = VGet(100.0f, -10.0f, 100.0f);
    transform_.scl = DEFAULT_SCALE;

    transform_.Update();

    mMaterial_ = std::make_unique<ModelMaterial>(
        "Shogiban_VS.cso",
        0,
        "Shogiban_PS.cso",
        1
    );

    mMaterial_->SetTextureBuf(0, 
        { resMng_.Load(ResourceManager::SRC::MINISHOGI_TEXTURE_UV).handleId_});
    mMaterial_->AddConstBufPS(FLOAT4{ 0.147f, 0.104f, 0.161f, 1.0f });

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
    /*mRenderer_->Draw();*/

    SetDrawMode(DX_DRAWMODE_BILINEAR);
    MV1DrawModel(transform_.modelId);
    SetDrawMode(DX_DRAWMODE_NUM);
}
