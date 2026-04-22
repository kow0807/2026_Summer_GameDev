#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
{

}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);


	// 確認用
	// シーン識別できるなら消してよし
	SetFontSize(20);
}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	if(ins.IsNew(KEY_INPUT_SPACE) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		printfDx("通過\n");
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void TitleScene::Draw(void)
{
}

void TitleScene::DrawUI(void)
{
	DrawFormatString(Application::SCREEN_SIZE_X / 2 - 100, Application::SCREEN_SIZE_Y / 2 - 20, GetColor(255, 255, 255), "Title");
}
