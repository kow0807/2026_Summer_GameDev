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
	: miniState_(MINI_STATE::FIRST_PRESS),
	selectState_(SELECT_STATE::GAME_SELECT),
	isYes_(true)
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
	switch (selectState_)
	{
	case SELECT_STATE::GAME_SELECT:
		SelectGameUpdate();
		break;

	case SELECT_STATE::EXPLANATION:
		ExplanationUpdate();
		break;

	case SELECT_STATE::PLAYING:
		GameUpdate();
		break;
	}
}

void GameScene::Draw(void)
{
	switch (selectState_)
	{
	case SELECT_STATE::GAME_SELECT:
		break;

	case SELECT_STATE::EXPLANATION:
		break;

	case SELECT_STATE::PLAYING:
		DrawGame();
		break;
	}
}

void GameScene::DrawUI(void)
{
	switch (selectState_)
	{
	case SELECT_STATE::GAME_SELECT:
		DrawFormatString(
			Application::SCREEN_SIZE_X / 2 - 100,
			Application::SCREEN_SIZE_Y / 2 - 150,
			GetColor(255, 255, 255),
			"ミニゲーム選ぶぜ！！"
		);
		SelectGameDrawUI();
		break;

	case SELECT_STATE::EXPLANATION:
		ExplanationDrawUI();
		break;

	case SELECT_STATE::PLAYING:
		DrawFormatString(100, 100, GetColor(255, 255, 255), "ゲーム中！");
		break;
	}
}

void GameScene::SelectGameUpdate(void)
{
	InputManager& ins = InputManager::GetInstance();

	int index = static_cast<int>(miniState_);
	const int max = static_cast<int>(MINI_STATE::MAX);

	if (ins.IsTrgDown(KEY_INPUT_RIGHT))
	{
		index++;
		if (index >= max) index = 0;
	}

	if (ins.IsTrgDown(KEY_INPUT_LEFT))
	{
		index--;
		if (index < 0) index = max - 1;
	}

	miniState_ = static_cast<MINI_STATE>(index);

	// 決定 → 説明画面へ
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		selectState_ = SELECT_STATE::EXPLANATION;
		isYes_ = true; // 初期は「はい」
	}
}

void GameScene::ExplanationUpdate()
{
	InputManager& ins = InputManager::GetInstance();

	// 左右で YES / NO 切り替え
	if (ins.IsTrgDown(KEY_INPUT_LEFT) || ins.IsTrgDown(KEY_INPUT_RIGHT))
	{
		isYes_ = !isYes_;
	}

	// 決定
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		if (isYes_)
		{
			selectState_ = SELECT_STATE::PLAYING;
		}
		else
		{
			selectState_ = SELECT_STATE::GAME_SELECT;
		}
	}
}

void GameScene::GameUpdate()
{
	InputManager& ins = InputManager::GetInstance();

	// 仮：スペースで戻る
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		selectState_ = SELECT_STATE::GAME_SELECT;
	}

	switch (miniState_)
	{
	case MINI_STATE::FIRST_PRESS:
		break;

	case MINI_STATE::QUIZ:
		break;

	case MINI_STATE::REVERSI:
		break;

	case MINI_STATE::BUTTON_MASH:
		break;

	case MINI_STATE::FLASH_CALC:
		break;

	case MINI_STATE::QUORIDOR:
		break;

	case MINI_STATE::HARE_AND_HOUNDS:
		break;

	case MINI_STATE::MINI_SHOGI:
		break;
	}
}

void GameScene::DrawGame()
{
	switch (miniState_)
	{
	case MINI_STATE::FIRST_PRESS:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "早押しゲーム");
		break;

	case MINI_STATE::QUIZ:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "クイズゲーム");
		break;

	case MINI_STATE::REVERSI:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "オセロ");
		break;

	case MINI_STATE::BUTTON_MASH:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "連打");
		break;

	case MINI_STATE::FLASH_CALC:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "フラッシュ暗算");
		break;

	case MINI_STATE::QUORIDOR:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "コリドール");
		break;

	case MINI_STATE::HARE_AND_HOUNDS:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "ウサギと猟犬");
		break;

	case MINI_STATE::MINI_SHOGI:
		DrawFormatString(100, 200, GetColor(255, 255, 255), "5五将棋");
		break;
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

	const int current = static_cast<int>(miniState_);

	for (int i = 0; i < static_cast<int>(MINI_STATE::MAX); i++)
	{
		int diff = i - current;

		// ループ補正
		if (diff > static_cast<int>(MINI_STATE::MAX) / 2)
			diff -= static_cast<int>(MINI_STATE::MAX);
		if (diff < -static_cast<int>(MINI_STATE::MAX) / 2)
			diff += static_cast<int>(MINI_STATE::MAX);

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

void GameScene::ExplanationDrawUI()
{
	const char* desc[] =
	{
		"早押しゲームです",
		"四択クイズです",
		"オセロです",
		"連打対決！",
		"フラッシュ暗算",
		"コリドール",
		"ウサギと猟犬",
		"5五将棋"
	};

	int index = static_cast<int>(miniState_);

	DrawFormatString(300, 200, GetColor(255, 255, 255), desc[index]);

	int yesColor = isYes_ ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
	int noColor = !isYes_ ? GetColor(255, 255, 0) : GetColor(255, 255, 255);

	DrawString(300, 300, "はい", yesColor);
	DrawString(450, 300, "いいえ", noColor);
}