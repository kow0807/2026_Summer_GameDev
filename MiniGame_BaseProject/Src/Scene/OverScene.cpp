#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "OverScene.h"

OverScene::OverScene(void)
{
}

OverScene::~OverScene(void)
{
}

void OverScene::Init(void)
{
	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);

}

void OverScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	// 入力受付（Enterキーでタイトルに戻る）
	if (InputManager::GetInstance().IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		printfDx("通過\n");
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void OverScene::Draw(void)
{
}

void OverScene::DrawUI(void)
{
	DrawFormatString(Application::SCREEN_SIZE_X / 2 - 100, Application::SCREEN_SIZE_Y / 2 - 20, GetColor(255, 255, 255), "Game Over");
}
