#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "ClearScene.h"

ClearScene::ClearScene(void)
{
}

ClearScene::~ClearScene(void)
{
}

void ClearScene::Init(void)
{

	// ’è“_ƒJƒƒ‰
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);

}

void ClearScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void ClearScene::Draw(void)
{
}

void ClearScene::DrawUI(void)
{
	DrawFormatString(Application::SCREEN_SIZE_X / 2 - 100, Application::SCREEN_SIZE_Y / 2 - 20, GetColor(255, 255, 255), "Clear Scene");
}
