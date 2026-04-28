#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Utility/AsoUtility.h"
#include "../../Renderer/ModelMaterial.h"
#include "../../Renderer/ModelRenderer.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
	: resMng_(ResourceManager::GetInstance()),
	scnMng_(SceneManager::GetInstance())
{
}

ActorBase::~ActorBase(void)
{
	transform_.Release();
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}

void ActorBase::InitModel(VECTOR pos, VECTOR scl, VECTOR quaRotLocal)
{
    // モデルの設定
    transform_.pos = { pos.x,pos.y,pos.z };
    transform_.scl = { scl.x,scl.y,scl.z };
    transform_.quaRot = Quaternion();
    transform_.quaRotLocal = Quaternion::Euler({ AsoUtility::Deg2RadF(quaRotLocal.x),
        AsoUtility::Deg2RadF(quaRotLocal.y),AsoUtility::Deg2RadF(quaRotLocal.z) });

    // モデル情報の更新
    transform_.Update();
}
