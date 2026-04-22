#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{

	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);
}

void GameScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	// いったん
 	if(ins.IsNew(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		printfDx("通過\n");
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	
	}

}

void GameScene::Draw(void)
{

}

void GameScene::DrawUI(void)
{
	DrawFormatString(Application::SCREEN_SIZE_X / 2 - 100, Application::SCREEN_SIZE_Y / 2 - 20, GetColor(255, 255, 255), "ミニゲーム選ぶぜ！！");
}
