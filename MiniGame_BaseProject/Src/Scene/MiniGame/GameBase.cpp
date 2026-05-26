#include "../../Manager/ResourceManager.h"
#include "../../Manager/Setting.h"
#include "GameBase.h"

GameBase::GameBase(void)
	:
	resMng_(ResourceManager::GetInstance()),
	setting_(Setting::GetInstance()),
	isReturn_(false)
{
}

GameBase::~GameBase(void)
{
}

bool GameBase::GetIsReturn(void)
{
	return isReturn_;
}

void GameBase::SetIsReturn(bool isReturn)
{
	isReturn_ = isReturn;
}
