#include "../../Manager/ResourceManager.h"
#include "../../Manager/Setting.h"
#include "GameBase.h"

GameBase::GameBase(void)
	:
	resMng_(ResourceManager::GetInstance()),
	setting_(Setting::GetInstance())
{
}

GameBase::~GameBase(void)
{
}