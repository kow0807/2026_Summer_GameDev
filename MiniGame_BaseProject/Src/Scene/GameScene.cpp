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
	: mState_(M_STATE::FIRST_PRESS)
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

	// シーン遷移（例）
	if (ins.IsNew(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN))
	{
		printfDx("通過\n");
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	SelectGameUpdate();

	switch (mState_)
	{
	case M_STATE::FIRST_PRESS:
		break;

	case M_STATE::QUIZ:
		break;

	case M_STATE::REVERSI:
		break;

	case M_STATE::BUTTON_MASH:
		break;

	case M_STATE::FLASH_CALC:
		break;

	case M_STATE::QUORIDOR:
		break;

	case M_STATE::HARE_AND_HOUNDS:
		break;

	case M_STATE::MINI_SHOGI:
		break;
	}

}

void GameScene::Draw(void)
{
	switch (mState_)
	{
	case M_STATE::FIRST_PRESS:
		break;

	case M_STATE::QUIZ:
		break;

	case M_STATE::REVERSI:
		break;

	case M_STATE::BUTTON_MASH:
		break;

	case M_STATE::FLASH_CALC:
		break;

	case M_STATE::QUORIDOR:
		break;

	case M_STATE::HARE_AND_HOUNDS:
		break;

	case M_STATE::MINI_SHOGI:
		break;
	}
}

void GameScene::DrawUI(void)
{
	DrawFormatString(
		Application::SCREEN_SIZE_X / 2 - 100,
		Application::SCREEN_SIZE_Y / 2 - 100,
		GetColor(255, 255, 255),
		"ミニゲーム選ぶぜ！！"
	);

	SelectGameDrawUI();
}

void GameScene::SelectGameUpdate(void)
{
	InputManager& ins = InputManager::GetInstance();

	int index = static_cast<int>(mState_);
	const int max = static_cast<int>(M_STATE::MAX);

	// 右
	if (ins.IsTrgDown(KEY_INPUT_RIGHT))
	{
		index++;
		if (index >= max) index = 0;
	}

	// 左
	if (ins.IsTrgDown(KEY_INPUT_LEFT))
	{
		index--;
		if (index < 0) index = max - 1;
	}

	mState_ = static_cast<M_STATE>(index);

	// 決定
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		printfDx("選択: %d\n", index);	//デバック用

		switch (mState_)
		{
		case M_STATE::FIRST_PRESS:
			break;

		case M_STATE::QUIZ:
			break;

		case M_STATE::REVERSI:
			break;

		case M_STATE::BUTTON_MASH:
			break;

		case M_STATE::FLASH_CALC:
			break;

		case M_STATE::QUORIDOR:
			break;

		case M_STATE::HARE_AND_HOUNDS:
			break;

		case M_STATE::MINI_SHOGI:
			break;
		}
	}
}

void GameScene::SelectGameDrawUI(void)
{
	const int centerX = Application::SCREEN_SIZE_X / 2;
	const int centerY = Application::SCREEN_SIZE_Y / 2;

	const int cardW = 200;
	const int cardH = 120;
	const int spacing = 260;

	const char* names[] =
	{
		"早押し",
		"四択クイズ",
		"オセロ",
		"連打対決",
		"フラッシュ暗算",
		"コリドール",
		"ウサギと猟犬",
		"5五将棋"
	};

	const int current = static_cast<int>(mState_);

	for (int i = 0; i < static_cast<int>(M_STATE::MAX); i++)
	{
		int diff = i - current;

		// ループ補正
		if (diff > static_cast<int>(M_STATE::MAX) / 2)
			diff -= static_cast<int>(M_STATE::MAX);
		if (diff < -static_cast<int>(M_STATE::MAX) / 2)
			diff += static_cast<int>(M_STATE::MAX);

		int x = centerX + diff * spacing;
		int y = centerY;

		bool isSelect = (i == current);

		int color = isSelect ?
			GetColor(255, 255, 0) :
			GetColor(200, 200, 200);

		// 枠
		DrawBox(
			x - cardW / 2, y - cardH / 2,
			x + cardW / 2, y + cardH / 2,
			color,
			FALSE
		);

		// 選択強調
		if (isSelect)
		{
			DrawBox(
				x - cardW / 2, y - cardH / 2,
				x + cardW / 2, y + cardH / 2,
				GetColor(255, 255, 0),
				TRUE
			);
		}

		// 文字
		int textW = GetDrawStringWidth(names[i], static_cast<int>(strlen(names[i])));

		DrawString(
			x - textW / 2,
			y + cardH / 2 + 10,
			names[i],
			GetColor(255, 255, 255)
		);
	}
}