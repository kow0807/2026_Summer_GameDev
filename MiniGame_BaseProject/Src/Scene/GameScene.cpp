#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Setting.h"
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"
#include "../Scene/MiniGame/GameBase.h"
#include "../Scene/MiniGame/FirstPressGame.h"
#include "../Scene/MiniGame/ButtonMashGame.h"
#include "../Scene/MiniGame/QuizGame.h"
#include "../Scene/MiniGame/Reversi.h"
#include "../Scene/MiniGame/Quoridor.h"
#include "../Scene/MiniGame/MiniShogi.h"
#include "GameScene.h"

namespace
{
	float g_loadingAnim = 0.0f;
}
	
GameScene::GameScene(void)
	: 
	miniState_(MINI_STATE::FIRST_PRESS),
	selectState_(SELECT_STATE::GAME_SELECT),
	isYes_(true),
	isKeyboard_(true),
	fadeAlpha_(0)
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

	explanationFontHandle_ = CreateFontToHandle(
		"游明朝",
		18,
		3);
	gameTitleFont_ = CreateFontToHandle(
		"游明朝",
		40,
		5,
		DX_FONTTYPE_ANTIALIASING_EDGE
	);

	backImg_ = resMng_.Load(ResourceManager::SRC::SELECT_BACK).handleId_;

	gameThumbnail_[0] = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_EXPLANATION).handleId_;
	gameThumbnail_[1] = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_EXPLANATION).handleId_;
	gameThumbnail_[2] = resMng_.Load(ResourceManager::SRC::REVERSI_EXPLANATION).handleId_;
	gameThumbnail_[3] = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_EXPLANATION).handleId_;
	gameThumbnail_[4] = resMng_.Load(ResourceManager::SRC::KARI).handleId_;
	gameThumbnail_[5] = resMng_.Load(ResourceManager::SRC::KARI).handleId_;

	firstPressExplanationImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_EXPLANATION).handleId_;
	quizExplanationImg_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_EXPLANATION).handleId_;
	reversiExplanationImg_ = resMng_.Load(ResourceManager::SRC::REVERSI_EXPLANATION).handleId_;
	buttonMashExplanationImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_EXPLANATION).handleId_;
	decideSEH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_DICIDE_SE).handleId_;

	isBgm_ = true;
	bgm_ = resMng_.Load(ResourceManager::SRC::SELECT_BGM).handleId_;
	ChangeVolumeSoundMem(190, bgm_);

	moveSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MOVE_SE).handleId_;
	cancelSe_ = resMng_.Load(ResourceManager::SRC::SELECT_CANCEL_SE).handleId_;
	selectSe_ = resMng_.Load(ResourceManager::SRC::SELECT_SELECT_SE).handleId_;
	menuSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MENU_SE).handleId_;

	leftArrowAnim_ = 0;
	rightArrowAnim_ = 0;

	isPause_ = false;
	pauseScreenHandle_ = MakeScreen(
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		TRUE);
	pauseX_ = -320.0f;
	pauseSelect_ = 0;
}

void GameScene::Update(void)
{
	if (isBgm_)
	{
		PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
		isBgm_ = false;
	}

	switch (selectState_)
	{
	case SELECT_STATE::GAME_SELECT:
		if (PauseUpdate())
		{
			return;
		}
		SelectGameUpdate();
		break;

	case SELECT_STATE::EXPLANATION:
		if (PauseUpdate())
		{
			return;
		}
		ExplanationUpdate();
		break;

	case SELECT_STATE::RUNTIME_LOADING:
		{
			CreateMiniGame();
			selectState_ = SELECT_STATE::PLAYING;
		}
		break;

	case SELECT_STATE::TRANSITION_OUT:
	{
		fadeAlpha_ += 8;

		if (fadeAlpha_ >= 255)
		{
			fadeAlpha_ = 255;
			CreateMiniGame();
			selectState_ = SELECT_STATE::TRANSITION_IN;
		}
	}
		break;
	case SELECT_STATE::TRANSITION_IN:
	{
		fadeAlpha_ -= 8;
		if (fadeAlpha_ <= 0)
		{
			fadeAlpha_ = 0;
			selectState_ = SELECT_STATE::PLAYING;
		}
	}
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
	case SELECT_STATE::RUNTIME_LOADING:
		//DrawRunTimeLoading();
		break;
	case SELECT_STATE::TRANSITION_OUT:
		ExplanationDrawUI();
		break;
	case SELECT_STATE::TRANSITION_IN:
		DrawGame();
		break;
	case SELECT_STATE::PLAYING:
		DrawGame();
		break;
	}
	DrawFade();
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
		if (gameBase_)
		{
			gameBase_->DrawUI();
		}
		break;
	}

	// パネルが見えている間だけ描画
	if (isPause_ || pauseX_ > -320.0f)
	{
		PauseDraw();
	}
}

void GameScene::SelectGameUpdate(void)
{
	InputManager& ins = InputManager::GetInstance();

	isKeyboard_ = true;

	int index = static_cast<int>(miniState_);
	const int max = static_cast<int>(MINI_STATE::MAX);

	// アニメーションタイマー更新
	if (leftArrowAnim_ > 0) leftArrowAnim_--;
	if (rightArrowAnim_ > 0) rightArrowAnim_--;

	if (ins.IsTrgDown(KEY_INPUT_RIGHT) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_RIGHT))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);

		index++;
		if (index >= max) index = 0;

		rightArrowAnim_ = 7;
	}

	if (ins.IsTrgDown(KEY_INPUT_LEFT) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_LEFT))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);

		index--;
		if (index < 0) index = max - 1;

		leftArrowAnim_ = 7;
	}

	miniState_ = static_cast<MINI_STATE>(index);

	// 決定 → 説明画面へ
	if (ins.IsTrgDown(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		PlaySoundMem(selectSe_, DX_PLAYTYPE_BACK);

		selectState_ = SELECT_STATE::EXPLANATION;
		isYes_ = true;
	}
}

void GameScene::ExplanationUpdate()
{
	InputManager& ins = InputManager::GetInstance();

	// 左右で YES / NO 切り替え
	if (ins.IsTrgDown(KEY_INPUT_LEFT) || ins.IsTrgDown(KEY_INPUT_RIGHT)
		|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_LEFT)
		|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_RIGHT))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);
		isYes_ = !isYes_;
	}

	// 上下で説明切り替え
	if (ins.IsTrgDown(KEY_INPUT_UP) || ins.IsTrgDown(KEY_INPUT_DOWN)
		|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP)
		|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);
		isKeyboard_ = !isKeyboard_;
	}

	// 決定
	if (ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		if (isYes_)
		{
			StopSoundMem(bgm_);
			PlaySoundMem(decideSEH_, DX_PLAYTYPE_BACK);
			fadeAlpha_ = 0;
			selectState_ = SELECT_STATE::TRANSITION_OUT;
		}
		else
		{
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
			selectState_ = SELECT_STATE::GAME_SELECT;
		}
	}
}

void GameScene::GameUpdate()
{
	InputManager& ins = InputManager::GetInstance();

	if (!gameBase_) return;

	if (gameBase_->GetIsReturn())
	{
		isBgm_ = true;
		//ミニゲームのリセット
		gameBase_->Reset();
		gameBase_->SetIsReturn(false);

		// ポインター破棄
		gameBase_.reset();
		selectState_ = SELECT_STATE::GAME_SELECT;
	}

	if(!gameBase_)
	{
		return;
	}
	gameBase_->Update();
}

bool GameScene::PauseUpdate(void)
{
	InputManager& ins = InputManager::GetInstance();

	//--------------------------------------
	// Escapeで開閉
	//--------------------------------------
	if (ins.IsTrgDown(KEY_INPUT_ESCAPE) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::SEVEN))
	{
		isPause_ = !isPause_;

		if (isPause_)
		{
			// ポーズ直前の画面を保存
			GetDrawScreenGraph(
				0,
				0,
				Application::SCREEN_SIZE_X,
				Application::SCREEN_SIZE_Y,
				pauseScreenHandle_);

			PlaySoundMem(menuSe_, DX_PLAYTYPE_BACK);
			StopSoundMem(bgm_);
		}
		else
		{
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
			PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP, false);
		}

		// 開いた・閉じた瞬間は入力を消費
		return true;
	}

	//--------------------------------------
	// スライドアニメーション
	//--------------------------------------
	float targetX = isPause_ ? 0.0f : -320.0f;
	pauseX_ += (targetX - pauseX_) * 0.2f;

	//--------------------------------------
	// ポーズ中でなければゲームへ入力を渡す
	//--------------------------------------
	if (!isPause_)
		return false;

	//--------------------------------------
	// カーソル移動
	//--------------------------------------
	if (ins.IsTrgDown(KEY_INPUT_UP) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);

		pauseSelect_--;

		if (pauseSelect_ < 0)
			pauseSelect_ = 2;
	}

	if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);
		
		pauseSelect_++;

		if (pauseSelect_ > 2)
			pauseSelect_ = 0;
	}

	//--------------------------------------
	// 決定
	//--------------------------------------
	if (ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		switch (pauseSelect_)
		{
		case 0:
			isPause_ = false;
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
			PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP, false);
			return true;    // このフレームはゲームへ入力を渡さない

		case 1:
			PlaySoundMem(decideSEH_, DX_PLAYTYPE_BACK);
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
			return true;

		case 2:
			SceneManager::GetInstance().SetGameEnd(true);
			return true;
		}
	}

	// ポーズ中は常に入力を消費
	return true;
}

void GameScene::PauseDraw(void)
{
	//--------------------------------------
	// 背景（ポーズ中だけ）
	//--------------------------------------
	if (!isPause_)
		return;


	//--------------------------------------
	// ポーズ直前の画面
	//--------------------------------------
	DrawGraph(
		0,
		0,
		pauseScreenHandle_,
		TRUE);


	//--------------------------------------
	// 暗いフィルター
	//--------------------------------------
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

	DrawBox(
		0,
		0,
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		GetColor(0, 0, 0),
		TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//--------------------------------------
	// パネルが画面外なら描画しない
	//--------------------------------------
	if (pauseX_ <= -320.0f)
		return;

	//--------------------------------------
	// パネル影
	//--------------------------------------
	DrawBox(
		(int)pauseX_ + 8,
		8,
		(int)pauseX_ + 308,
		Application::SCREEN_SIZE_Y,
		GetColor(20, 20, 20),
		TRUE);

	//--------------------------------------
	// パネル本体
	//--------------------------------------
	DrawBox(
		(int)pauseX_,
		0,
		(int)pauseX_ + 300,
		Application::SCREEN_SIZE_Y,
		GetColor(35, 35, 45),
		TRUE);

	//--------------------------------------
	// 枠
	//--------------------------------------
	DrawBox(
		(int)pauseX_ - 100,
		0,
		(int)pauseX_ + 300,
		Application::SCREEN_SIZE_Y,
		GetColor(0, 220, 255),
		FALSE);

	//--------------------------------------
	// タイトル
	//--------------------------------------
	DrawStringToHandle(
		(int)pauseX_ + 30,
		40,
		"PAUSE",
		GetColor(255, 255, 255),
		explanationFontHandle_);

	DrawLine(
		(int)pauseX_ + 30,
		75,
		(int)pauseX_ + 270,
		75,
		GetColor(0, 220, 255));

	//--------------------------------------
	// メニュー
	//--------------------------------------
	const char* menu[3] =
	{
		"ゲームに戻る",
		"タイトルに戻る",
		"ゲーム終了"
	};

	for (int i = 0; i < 3; i++)
	{
		int y = 150 + i * 80;

		if (i == pauseSelect_)
		{
			DrawBox(
				(int)pauseX_ + 20,
				y - 8,
				(int)pauseX_ + 280,
				y + 30,
				GetColor(0, 180, 220),
				TRUE);

			DrawBox(
				(int)pauseX_ + 20,
				y - 8,
				(int)pauseX_ + 28,
				y + 30,
				GetColor(255, 255, 255),
				TRUE);

			DrawStringToHandle(
				(int)pauseX_ + 45,
				y,
				menu[i],
				GetColor(255, 255, 255),
				explanationFontHandle_);
		}
		else
		{
			DrawStringToHandle(
				(int)pauseX_ + 45,
				y,
				menu[i],
				GetColor(180, 180, 180),
				explanationFontHandle_);
		}
	}

	//--------------------------------------
	// 操作説明
	//--------------------------------------
	DrawLine(
		(int)pauseX_ + 20,
		500,
		(int)pauseX_ + 280,
		500,
		GetColor(80, 80, 80));

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameScene::DrawGame()
{
	if (!gameBase_)
	{
		return;
	}

	gameBase_->Draw();
}

void GameScene::SelectGameDrawUI()
{
	const int centerX = Application::SCREEN_SIZE_X / 2;
	const int centerY = Application::SCREEN_SIZE_Y / 2;

	const int offsetY = -20;

	//--------------------------------
	// 背景
	//--------------------------------
	DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, TRUE);

	const char* names[] =
	{
		"早押し",
		"四択クイズ",
		"オセロ",
		"連打対決",
		"コリドール",
		"5五将棋"
	};

	int index = static_cast<int>(miniState_);

	const int width = 800;
	const int height = 500;

	int left = centerX - width / 2;
	int right = centerX + width / 2;
	int top = centerY - height / 2 + offsetY;
	int bottom = centerY + height / 2 + offsetY;

	//--------------------------------
	// サムネイル
	//--------------------------------
	DrawExtendGraph(
		left,
		top,
		right,
		bottom,
		gameThumbnail_[index],
		TRUE);

	//--------------------------------
	// 外側の発光枠
	//--------------------------------
	DrawBox(left - 4, top - 4, right + 4, bottom + 4, GetColor(0, 180, 255), FALSE);
	DrawBox(left - 2, top - 2, right + 2, bottom + 2, GetColor(100, 220, 255), FALSE);
	DrawBox(left, top, right, bottom, GetColor(255, 255, 255), FALSE);

	//--------------------------------
	// 四隅装飾
	//--------------------------------
	const int c = 30;

	DrawLine(left, top, left + c, top, GetColor(0, 220, 255));
	DrawLine(left, top, left, top + c, GetColor(0, 220, 255));

	DrawLine(right, top, right - c, top, GetColor(0, 220, 255));
	DrawLine(right, top, right, top + c, GetColor(0, 220, 255));

	DrawLine(left, bottom, left + c, bottom, GetColor(0, 220, 255));
	DrawLine(left, bottom, left, bottom - c, GetColor(0, 220, 255));

	DrawLine(right, bottom, right - c, bottom, GetColor(0, 220, 255));
	DrawLine(right, bottom, right, bottom - c, GetColor(0, 220, 255));

	//--------------------------------
	// タイトル
	//--------------------------------
	int textW = GetDrawStringWidthToHandle(
		names[index],
		strlen(names[index]),
		gameTitleFont_);

	int textX = centerX - textW / 2;
	int textY = bottom + 30;

	DrawStringToHandle(textX - 2, textY, names[index], GetColor(0, 180, 255), gameTitleFont_);
	DrawStringToHandle(textX + 2, textY, names[index], GetColor(0, 180, 255), gameTitleFont_);
	DrawStringToHandle(textX, textY - 2, names[index], GetColor(0, 180, 255), gameTitleFont_);
	DrawStringToHandle(textX, textY + 2, names[index], GetColor(0, 180, 255), gameTitleFont_);

	DrawStringToHandle(textX + 3, textY + 3, names[index], GetColor(0, 0, 0), gameTitleFont_);
	DrawStringToHandle(textX, textY, names[index], GetColor(255, 255, 255), gameTitleFont_);

	//--------------------------------
	// 点滅する矢印（押したら拡大＋色変更）
	//--------------------------------
	const int arrowOffset = width / 2 + 60;

	int alpha = 120 + static_cast<int>(135 * (sin(GetNowCount() * 0.005) + 1.0) * 0.5);

	//==========================
	// 左矢印
	//==========================
	{
		int lx = centerX - arrowOffset;
		int ly = centerY + offsetY;

		int w = (leftArrowAnim_ > 0) ? 30 : 24;
		int h = (leftArrowAnim_ > 0) ? 36 : 30;

		int fillColor = (leftArrowAnim_ > 0)
			? GetColor(255, 230, 0)		// 黄色
			: GetColor(255, 255, 255);	// 白

		int frameColor = (leftArrowAnim_ > 0)
			? GetColor(255, 120, 0)		// オレンジ
			: GetColor(0, 220, 255);	// シアン

		SetDrawBlendMode(DX_BLENDMODE_ALPHA,
			(leftArrowAnim_ > 0) ? 255 : alpha);

		// 中身
		DrawTriangle(
			lx - w, ly,
			lx + w, ly - h,
			lx + w, ly + h,
			fillColor,
			TRUE);

		// 白枠
		DrawTriangle(
			lx - w, ly,
			lx + w, ly - h,
			lx + w, ly + h,
			GetColor(255, 255, 255),
			FALSE);

		// 外枠
		DrawTriangle(
			lx - w - 4, ly,
			lx + w + 4, ly - h - 5,
			lx + w + 4, ly + h + 5,
			frameColor,
			FALSE);
	}

	//==========================
	// 右矢印
	//==========================
	{
		int rx = centerX + arrowOffset;
		int ry = centerY + offsetY;

		int w = (rightArrowAnim_ > 0) ? 30 : 24;
		int h = (rightArrowAnim_ > 0) ? 36 : 30;

		int fillColor = (rightArrowAnim_ > 0)
			? GetColor(255, 230, 0)
			: GetColor(255, 255, 255);

		int frameColor = (rightArrowAnim_ > 0)
			? GetColor(255, 120, 0)
			: GetColor(0, 220, 255);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA,
			(rightArrowAnim_ > 0) ? 255 : alpha);

		// 中身
		DrawTriangle(
			rx + w, ry,
			rx - w, ry - h,
			rx - w, ry + h,
			fillColor,
			TRUE);

		// 白枠
		DrawTriangle(
			rx + w, ry,
			rx - w, ry - h,
			rx - w, ry + h,
			GetColor(255, 255, 255),
			FALSE);

		// 外枠
		DrawTriangle(
			rx + w + 4, ry,
			rx - w - 4, ry - h - 5,
			rx - w - 4, ry + h + 5,
			frameColor,
			FALSE);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameScene::ExplanationDrawUI()
{
	switch (miniState_)
	{
	case MINI_STATE::FIRST_PRESS:
		ExplanationFirstPressDrawUI();
		break;
	case MINI_STATE::QUIZ:
		ExplanationQuizDrawUI();
		break;
	case MINI_STATE::REVERSI:
		ExplanationReversiDrawUI();
		break;
	case MINI_STATE::BUTTON_MASH:
		ExplanationButtonMashDrawUI();
		break;
	case MINI_STATE::QUORIDOR:
		ExplanationQuoridorDrawUI();
		return;
		break;
	case MINI_STATE::MINI_SHOGI:
		ExplanationMiniShogiUI();
		break;
	}
}

void GameScene::ExplanationFirstPressDrawUI(void)
{
	int screenX = Application::SCREEN_SIZE_X;
	int screenY = Application::SCREEN_SIZE_Y;

	int centerX = screenX / 2;
	int centerY = screenY / 2;

	// 背景暗転
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);

	DrawBox(0, 0, screenX, screenY, GetColor(0, 0, 0), true);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// メインパネル
	int panelW = 1200;
	int panelH = 650;

	int panelX = centerX - panelW / 2;
	int panelY = centerY - panelH / 2;

	// 外枠
	DrawBox(panelX - 3, panelY - 3, panelX + panelW + 3, panelY + panelH + 3, GetColor(0, 255, 255), true);

	// 内側
	DrawBox(panelX, panelY, panelX + panelW, panelY + panelH, GetColor(12, 18, 30), true);

	// タイトル
	DrawStringToHandle(panelX + 120, panelY + 20, "早押しゲーム", GetColor(0, 255, 255), explanationFontHandle_);

	// 区切り線
	DrawLine(panelX + 30, panelY + 70, panelX + panelW - 30, panelY + 70, GetColor(0, 180, 255));

	// 左側 : 説明画像
	int imageFrameX = panelX + 40;
	int imageFrameY = panelY + 110;

	int imageFrameW = 600;
	int imageFrameH = 390;

	// 画像背景
	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(20, 28, 45), true);

	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(0, 180, 255), false);

	// 枠中央
	int imageCenterX = imageFrameX + imageFrameW / 2;
	int imageCenterY = imageFrameY + imageFrameH / 2;

	// 説明画像
	DrawRotaGraph(imageCenterX, imageCenterY, 0.5, 0.0, firstPressExplanationImg_, true);

	// 右側 : ルール説明
	int textX = panelX + 700;
	int textY = panelY + 140;

	DrawStringToHandle(textX, textY, "■ ルール説明", GetColor(255, 255, 255), explanationFontHandle_);

	// ページ切替案内
	DrawStringToHandle(
		textX + 180,
		textY,
		"↑↓切替",
		GetColor(180, 180, 180),
		explanationFontHandle_);

	if (isKeyboard_)
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 1/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ キーボード ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・合図が表示された瞬間に", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 150, "Spaceキーを押してください", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 230, "・2本先取で勝利", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 310, "・フェイクに騙されないように", GetColor(255, 220, 120), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 350, "気を付けましょう", GetColor(255, 220, 120), explanationFontHandle_);
	}
	else
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 2/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ PAD ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・合図が表示された瞬間に", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 150, "Aボタンを押してください", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 230, "・2本先取で勝利", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 310, "・フェイクに騙されないように", GetColor(255, 220, 120), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 350, "気を付けましょう", GetColor(255, 220, 120), explanationFontHandle_);
	}

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, static_cast<int>(strlen(startText)), explanationFontHandle_);

	DrawStringToHandle(centerX - startTextW / 2, panelY + 560, startText, GetColor(255, 255, 255), explanationFontHandle_);


	// はい / いいえ
	int yesColor = isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);

	int noColor = !isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);


	// ボタン設定
	int buttonW = 180;
	int buttonH = 42;
	int buttonGap = 40;

	int totalW = buttonW * 2 + buttonGap;

	int startX = centerX - totalW / 2;
	int buttonY = panelY + 595;


	// はいボタン
	int yesX = startX;

	DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (isYes_)
	{
		DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* yesText = "はい";

	int yesTextW = GetDrawStringWidthToHandle(yesText, static_cast<int>(strlen(yesText)), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, static_cast<int>(strlen(noText)), explanationFontHandle_);

	DrawStringToHandle(noX + buttonW / 2 - noTextW / 2, buttonY + 12, noText, noColor, explanationFontHandle_);
}

void GameScene::ExplanationQuizDrawUI(void)
{
	int screenX = Application::SCREEN_SIZE_X;
	int screenY = Application::SCREEN_SIZE_Y;

	int centerX = screenX / 2;
	int centerY = screenY / 2;

	// 背景暗転
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);

	DrawBox(0, 0, screenX, screenY, GetColor(0, 0, 0), true);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// メインパネル
	int panelW = 1200;
	int panelH = 650;

	int panelX = centerX - panelW / 2;
	int panelY = centerY - panelH / 2;

	// 外枠
	DrawBox(panelX - 3, panelY - 3, panelX + panelW + 3, panelY + panelH + 3, GetColor(0, 255, 255), true);

	// 内側
	DrawBox(panelX, panelY, panelX + panelW, panelY + panelH, GetColor(12, 18, 30), true);

	// タイトル
	DrawStringToHandle(panelX + 120, panelY + 20, "四択クイズ", GetColor(0, 255, 255), explanationFontHandle_);

	// 区切り線
	DrawLine(panelX + 30, panelY + 70, panelX + panelW - 30, panelY + 70, GetColor(0, 180, 255));

	// 左側 : 説明画像
	int imageFrameX = panelX + 40;
	int imageFrameY = panelY + 110;

	int imageFrameW = 600;
	int imageFrameH = 390;

	// 画像背景
	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(20, 28, 45), true);

	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(0, 180, 255), false);

	// 枠中央
	int imageCenterX = imageFrameX + imageFrameW / 2;
	int imageCenterY = imageFrameY + imageFrameH / 2;

	// 説明画像
	DrawRotaGraph(imageCenterX, imageCenterY, 0.5, 0.0, quizExplanationImg_, true);

	// 右側 : ルール説明
	int textX = panelX + 700;
	int textY = panelY + 140;

	DrawStringToHandle(textX, textY, "■ ルール説明", GetColor(255, 255, 255), explanationFontHandle_);

	// ページ切替案内
	DrawStringToHandle(
		textX + 180,
		textY,
		"↑↓切替",
		GetColor(180, 180, 180),
		explanationFontHandle_);

	if (isKeyboard_)
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 1/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ キーボード ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・クイズに10問中8問", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 150, "正解でクリア", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 230, "・矢印キーで回答を選択", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 310, "・Enterキーで決定", GetColor(220, 220, 220), explanationFontHandle_);

	}
	else
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 2/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ PAD ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・クイズに10問中8問", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 150, "正解でクリア", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 230, "・十字ボタンで回答を選択", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 310, "・Aボタンで決定", GetColor(220, 220, 220), explanationFontHandle_);

	}

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, static_cast<int>(strlen(startText)), explanationFontHandle_);

	DrawStringToHandle(centerX - startTextW / 2, panelY + 560, startText, GetColor(255, 255, 255), explanationFontHandle_);


	// はい / いいえ
	int yesColor = isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);

	int noColor = !isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);


	// ボタン設定
	int buttonW = 180;
	int buttonH = 42;
	int buttonGap = 40;

	int totalW = buttonW * 2 + buttonGap;

	int startX = centerX - totalW / 2;
	int buttonY = panelY + 595;


	// はいボタン
	int yesX = startX;

	DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (isYes_)
	{
		DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* yesText = "はい";

	int yesTextW = GetDrawStringWidthToHandle(yesText, static_cast<int>(strlen(yesText)), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, static_cast<int>(strlen(noText)), explanationFontHandle_);

	DrawStringToHandle(noX + buttonW / 2 - noTextW / 2, buttonY + 12, noText, noColor, explanationFontHandle_);
}

void GameScene::ExplanationReversiDrawUI(void)
{
	int screenX = Application::SCREEN_SIZE_X;
	int screenY = Application::SCREEN_SIZE_Y;

	int centerX = screenX / 2;
	int centerY = screenY / 2;

	// 背景暗転
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);

	DrawBox(0, 0, screenX, screenY, GetColor(0, 0, 0), true);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// メインパネル
	int panelW = 1200;
	int panelH = 650;

	int panelX = centerX - panelW / 2;
	int panelY = centerY - panelH / 2;

	// 外枠
	DrawBox(panelX - 3, panelY - 3, panelX + panelW + 3, panelY + panelH + 3, GetColor(0, 255, 255), true);

	// 内側
	DrawBox(panelX, panelY, panelX + panelW, panelY + panelH, GetColor(12, 18, 30), true);

	// タイトル
	DrawStringToHandle(panelX + 120, panelY + 20, "オセロ", GetColor(0, 255, 255), explanationFontHandle_);

	// 区切り線
	DrawLine(panelX + 30, panelY + 70, panelX + panelW - 30, panelY + 70, GetColor(0, 180, 255));

	// 左側 : 説明画像
	int imageFrameX = panelX + 40;
	int imageFrameY = panelY + 110;

	int imageFrameW = 600;
	int imageFrameH = 390;

	// 画像背景
	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(20, 28, 45), true);

	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(0, 180, 255), false);

	// 枠中央
	int imageCenterX = imageFrameX + imageFrameW / 2;
	int imageCenterY = imageFrameY + imageFrameH / 2;

	// 説明画像
	DrawRotaGraph(imageCenterX, imageCenterY, 0.5, 0.0, reversiExplanationImg_, true);

	// 右側 : ルール説明
	int textX = panelX + 700;
	int textY = panelY + 140;

	DrawStringToHandle(textX, textY, "■ ルール説明", GetColor(255, 255, 255), explanationFontHandle_);

	// ページ切替案内
	DrawStringToHandle(
		textX + 180,
		textY,
		"↑↓切替",
		GetColor(180, 180, 180),
		explanationFontHandle_);

	if (isKeyboard_)
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 1/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ キーボード ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・矢印キーでマスを選択", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 170, "・Enterキーで決定", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 230, "・盤面が埋まるか", GetColor(220, 220, 220), explanationFontHandle_);
		DrawStringToHandle(textX + 25, textY + 270, "駒を置けなくなると終了", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 330, "・駒の多いほうの勝利", GetColor(220, 220, 220), explanationFontHandle_);
	}
	else
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 2/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ PAD ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・十字ボタンでマスを選択", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 170, "・Aボタンで決定", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 230, "・盤面が埋まるか", GetColor(220, 220, 220), explanationFontHandle_);
		DrawStringToHandle(textX + 25, textY + 270, "駒を置けなくなると終了", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 330, "・駒の多いほうの勝利", GetColor(220, 220, 220), explanationFontHandle_);
	}

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, static_cast<int>(strlen(startText)), explanationFontHandle_);

	DrawStringToHandle(centerX - startTextW / 2, panelY + 560, startText, GetColor(255, 255, 255), explanationFontHandle_);


	// はい / いいえ
	int yesColor = isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);

	int noColor = !isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);


	// ボタン設定
	int buttonW = 180;
	int buttonH = 42;
	int buttonGap = 40;

	int totalW = buttonW * 2 + buttonGap;

	int startX = centerX - totalW / 2;
	int buttonY = panelY + 595;


	// はいボタン
	int yesX = startX;

	DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (isYes_)
	{
		DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* yesText = "はい";

	int yesTextW = GetDrawStringWidthToHandle(yesText, static_cast<int>(strlen(yesText)), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, static_cast<int>(strlen(noText)), explanationFontHandle_);

	DrawStringToHandle(noX + buttonW / 2 - noTextW / 2, buttonY + 12, noText, noColor, explanationFontHandle_);
}

void GameScene::ExplanationButtonMashDrawUI(void)
{
	int screenX = Application::SCREEN_SIZE_X;
	int screenY = Application::SCREEN_SIZE_Y;

	int centerX = screenX / 2;
	int centerY = screenY / 2;

	// 背景暗転
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);

	DrawBox(0, 0, screenX, screenY, GetColor(0, 0, 0), true);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// メインパネル
	int panelW = 1200;
	int panelH = 650;

	int panelX = centerX - panelW / 2;
	int panelY = centerY - panelH / 2;

	// 外枠
	DrawBox(panelX - 3, panelY - 3, panelX + panelW + 3, panelY + panelH + 3, GetColor(0, 255, 255), true);

	// 内側
	DrawBox(panelX, panelY, panelX + panelW, panelY + panelH, GetColor(12, 18, 30), true);

	// タイトル
	DrawStringToHandle(panelX + 120, panelY + 20, "連打対決", GetColor(0, 255, 255), explanationFontHandle_);

	// 区切り線
	DrawLine(panelX + 30, panelY + 70, panelX + panelW - 30, panelY + 70, GetColor(0, 180, 255));

	// 左側 : 説明画像
	int imageFrameX = panelX + 40;
	int imageFrameY = panelY + 110;

	int imageFrameW = 600;
	int imageFrameH = 390;

	// 画像背景
	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(20, 28, 45), true);

	DrawBox(imageFrameX, imageFrameY, imageFrameX + imageFrameW, imageFrameY + imageFrameH, GetColor(0, 180, 255), false);

	// 枠中央
	int imageCenterX = imageFrameX + imageFrameW / 2;
	int imageCenterY = imageFrameY + imageFrameH / 2;

	// 説明画像
	DrawRotaGraph(imageCenterX, imageCenterY, 0.5, 0.0, buttonMashExplanationImg_, true);

	// 右側 : ルール説明
	int textX = panelX + 700;
	int textY = panelY + 140;

	DrawStringToHandle(textX, textY, "■ ルール説明", GetColor(255, 255, 255), explanationFontHandle_);

	// ページ切替案内
	DrawStringToHandle(
		textX + 180,
		textY,
		"↑↓切替",
		GetColor(180, 180, 180),
		explanationFontHandle_);

	if (isKeyboard_)
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 1/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ キーボード ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・合図が表示された後", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 150, "Spaceキーを連打してください", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 210, "・連打の優勢状況によって", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 250, "画面の色が変化します", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 310, "・青で埋め尽くすと1本獲得し", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 350, "2本先取で勝利", GetColor(220, 220, 220), explanationFontHandle_);
	}
	else
	{
		// ページ番号
		DrawStringToHandle(
			textX + 300,
			textY,
			"[ 2/2 ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

		DrawStringToHandle(
			textX,
			textY + 55,
			"[ PAD ]",
			GetColor(0, 255, 255),
			explanationFontHandle_);

		DrawStringToHandle(textX, textY + 110, "・合図が表示された後", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 150, "Aボタンを連打してください", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 210, "・連打の優勢状況によって", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 250, "画面の色が変化します", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX, textY + 310, "・青で埋め尽くすと1本獲得し", GetColor(220, 220, 220), explanationFontHandle_);

		DrawStringToHandle(textX + 25, textY + 350, "2本先取で勝利", GetColor(220, 220, 220), explanationFontHandle_);
	}

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, static_cast<int>(strlen(startText)), explanationFontHandle_);

	DrawStringToHandle(centerX - startTextW / 2, panelY + 560, startText, GetColor(255, 255, 255), explanationFontHandle_);


	// はい / いいえ
	int yesColor = isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);

	int noColor = !isYes_ ? GetColor(255, 255, 0) : GetColor(180, 180, 180);


	// ボタン設定
	int buttonW = 180;
	int buttonH = 42;
	int buttonGap = 40;

	int totalW = buttonW * 2 + buttonGap;

	int startX = centerX - totalW / 2;
	int buttonY = panelY + 595;


	// はいボタン
	int yesX = startX;

	DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (isYes_)
	{
		DrawBox(yesX, buttonY, yesX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* yesText = "はい";

	int yesTextW = GetDrawStringWidthToHandle(yesText, static_cast<int>(strlen(yesText)), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, static_cast<int>(strlen(noText)), explanationFontHandle_);

	DrawStringToHandle(noX + buttonW / 2 - noTextW / 2, buttonY + 12, noText, noColor, explanationFontHandle_);

}

void GameScene::ExplanationQuoridorDrawUI(void)
{
	int index = static_cast<int>(miniState_);

	// 🎨 画面解像度に応じた比率（スケール）の計算
	const auto& windowSize = Setting::GetInstance().GetWindowSize();
	float scaleX = static_cast<float>(windowSize.width_) / 1024.0f;
	float scaleY = static_cast<float>(windowSize.height_) / 640.0f;

	// 文字の大きさの基準（縦横の比率が極端に崩れないよう、小さい方の比率をベースにする）
	float fontScale = (scaleX < scaleY) ? scaleX : scaleY;
	if (fontScale < 0.1f) fontScale = 1.0f; // 安全対策

	// 色定義
	int titleColor = GetColor(240, 200, 80);	// 上品なアンティークゴールド
	int headerColor = GetColor(230, 210, 150); // 落ち着いたライトゴールド
	int textColor = GetColor(240, 240, 240); // 眩しすぎないオフホワイト
	int alertColor = GetColor(255, 130, 130); // 規則用のサーモンピンク
	int yellowColor = GetColor(255, 255, 150); // 注意を引く明るいイエロー
	int whiteColor = GetColor(240, 240, 240); // 説明文のオフホワイト

	int selectActiveColor = GetColor(255, 230, 100); // 選択中の鮮やかなゴールド
	int selectInactiveColor = GetColor(160, 180, 180); // 非選択時のくすんだシルバー

	// 📐 画面比率を掛けた基準座標（左端のライン）
	int baseX = static_cast<int>(180 * scaleX);
	int baseY = static_cast<int>(110 * scaleY);
	int lineSpace = static_cast<int>(28 * scaleY);

	int curY = baseY;

	// 📢 ルール説明文の描画（baseXを基準に美しく整列）
	DrawExtendString(baseX, curY, fontScale, fontScale, "【 コリドール (Quoridor) 概要 】", titleColor);
	curY += lineSpace * 2;

	DrawExtendString(baseX, curY, fontScale, fontScale, "■ 勝利条件", headerColor);
	curY += lineSpace;
	DrawExtendString(baseX, curY, fontScale, fontScale, "自分のコマ(赤)を、一番奥(最上段)の列へ相手(青)より先に到達させたら勝利！", whiteColor);
	curY += lineSpace * 2;

	DrawExtendString(baseX, curY, fontScale, fontScale, "■ あなたのターンでできること（どちらか1つ）", yellowColor);
	curY += lineSpace;
	DrawExtendString(baseX + static_cast<int>(20 * scaleX), curY, fontScale, fontScale, "【1】 コマの移動 : 上下左右に1マス動かせます。(敵と隣接時は飛び越し可能)", whiteColor);
	curY += lineSpace;
	DrawExtendString(baseX + static_cast<int>(20 * scaleX), curY, fontScale, fontScale, "【2】 壁の設置 : 残り壁を消費して、相手の進路を邪魔する壁を置けます。", whiteColor);
	curY += lineSpace * 2;

	DrawExtendString(baseX, curY, fontScale, fontScale, "注意：相手を完全に閉じ込める壁の配置は禁止です！(常にゴールへの道を1マス以上残す)", alertColor);
	curY += lineSpace * 3;

	// ──────────────────────────────────────────
	// 🔘 画像 image_85dec1.png のレイアウトを完全再現する部分
	// ──────────────────────────────────────────
	int yesColor = isYes_ ? selectActiveColor : selectInactiveColor;
	int noColor = !isYes_ ? selectActiveColor : selectInactiveColor;

	// 1. 質問文の描画：タイトルより少し右（インデント）にずらして配置
	DrawExtendString(baseX + static_cast<int>(100 * scaleX), curY, fontScale, fontScale, "このゲームを開始しますか？", textColor);

	// 下方向に移動
	curY += static_cast<int>(50 * scaleY);

	// 2. 「はい」の描画：★タイトルの左端（baseX）から少し右（例: +160px）の位置に固定
	// これにより、上の文章構造と完全に美しい縦ラインが形成されます。
	int yesX = baseX + static_cast<int>(160 * scaleX);
	DrawExtendString(yesX, curY, fontScale, fontScale, "はい", yesColor);

	// 3. 「いいえ」の描画：「はい」の開始位置からさらに右に一定の距離（例: +160px）離して配置
	int noX = yesX + static_cast<int>(160 * scaleX);
	DrawExtendString(noX, curY, fontScale, fontScale, "いいえ", noColor);
}

void GameScene::ExplanationMiniShogiUI(void)
{
	int index = static_cast<int>(miniState_);

	//----------------------------------
	// 画面サイズに応じたスケール
	//----------------------------------
	const auto& windowSize =
		Setting::GetInstance().GetWindowSize();

	float scaleX =
		static_cast<float>(windowSize.width_) /
		1024.0f;

	float scaleY =
		static_cast<float>(windowSize.height_) /
		640.0f;

	float fontScale =
		(scaleX < scaleY)
		? scaleX
		: scaleY;

	if (fontScale < 0.1f)
	{
		fontScale = 1.0f;
	}

	//----------------------------------
	// 色
	//----------------------------------
	const int titleColor =
		GetColor(240, 200, 80);

	const int headerColor =
		GetColor(230, 210, 150);

	const int textColor =
		GetColor(240, 240, 240);

	const int alertColor =
		GetColor(255, 130, 130);

	const int yellowColor =
		GetColor(255, 255, 150);

	const int whiteColor =
		GetColor(240, 240, 240);

	const int selectActiveColor =
		GetColor(255, 230, 100);

	const int selectInactiveColor =
		GetColor(160, 180, 180);

	//----------------------------------
	// 基準座標
	//----------------------------------
	const int baseX =
		static_cast<int>(150 * scaleX);

	const int baseY =
		static_cast<int>(50 * scaleY);

	const int lineSpace =
		static_cast<int>(20 * scaleY);

	int curY = baseY;

	//----------------------------------
	// タイトル
	//----------------------------------
	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"【 五々将棋 概要 】",
		titleColor);

	curY += lineSpace * 2;

	//----------------------------------
	// 勝利条件
	//----------------------------------
	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"■ 勝利条件",
		headerColor);

	curY += lineSpace;

	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"相手の王を逃げられない状態にする「詰み」にすれば勝利です。",
		whiteColor);

	curY += lineSpace * 2;

	//----------------------------------
	// ターン中にできること
	//----------------------------------
	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"■ あなたのターンでできること",
		yellowColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"【1】盤上の自分の駒を選び、移動可能なマスへ動かす",
		whiteColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"【2】持ち駒を選び、空いているマスへ配置する",
		whiteColor);

	curY += lineSpace * 2;

	//----------------------------------
	// 成り
	//----------------------------------
	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"■ 成り",
		headerColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"銀・角・飛車・歩は、相手陣に入るか相手陣から出るときに成れます。",
		whiteColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"歩が最終段へ進む場合は、必ず成ります。",
		whiteColor);

	curY += lineSpace * 2;

	//----------------------------------
	// 駒を取る
	//----------------------------------
	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"■ 駒を取る",
		headerColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"相手の駒があるマスへ移動すると、その駒を持ち駒として使用できます。",
		whiteColor);

	curY += lineSpace * 2;

	//----------------------------------
	// 注意事項
	//----------------------------------
	DrawExtendString(
		baseX,
		curY,
		fontScale,
		fontScale,
		"■ 禁止されている手",
		alertColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"・同じ縦列に、自分の成っていない歩を2枚置く「二歩」",
		alertColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"・歩を最終段へ打ち、動けない状態にする手",
		alertColor);

	curY += lineSpace;

	DrawExtendString(
		baseX + static_cast<int>(20 * scaleX),
		curY,
		fontScale,
		fontScale,
		"・自分の王が取られる状態になる手、または王手を放置する手",
		alertColor);

	curY += lineSpace * 2;

	//----------------------------------
	// 開始確認
	//----------------------------------
	const int yesColor =
		isYes_
		? selectActiveColor
		: selectInactiveColor;

	const int noColor =
		!isYes_
		? selectActiveColor
		: selectInactiveColor;

	DrawExtendString(
		baseX + static_cast<int>(100 * scaleX),
		curY,
		fontScale,
		fontScale,
		"このゲームを開始しますか？",
		textColor);

	curY += static_cast<int>(45 * scaleY);

	const int yesX =
		baseX +
		static_cast<int>(160 * scaleX);

	DrawExtendString(
		yesX,
		curY,
		fontScale,
		fontScale,
		"はい",
		yesColor);

	const int noX =
		yesX +
		static_cast<int>(160 * scaleX);

	DrawExtendString(
		noX,
		curY,
		fontScale,
		fontScale,
		"いいえ",
		noColor);
}

void GameScene::CreateMiniGame(void)
{
	// gameBase_のポインタ生成ができていないところを選択した場合
	// 中身がないので例外スローになる。追加する際はここに生成コードを追加せよ
	
	switch (miniState_)
	{
	case MINI_STATE::FIRST_PRESS:
		gameBase_ = std::make_unique<FirstPressGame>();
		break;
	case MINI_STATE::QUIZ:
		gameBase_ = std::make_unique<QuizGame>();
		break;
	case MINI_STATE::REVERSI:
		gameBase_ = std::make_unique<Reversi>();
		break;
	case MINI_STATE::BUTTON_MASH:
		gameBase_ = std::make_unique<ButtonMashGame>();
		break;
	case MINI_STATE::QUORIDOR:
		gameBase_ = std::make_unique<Quoridor>();
		break;
	case MINI_STATE::MINI_SHOGI:
		gameBase_ = std::make_unique<MiniShogi>();
		break;
	}

	gameBase_->Init();
}

void GameScene::DrawFade(void)
{
	if (fadeAlpha_ <= 0)
	{
		return;
	}

	auto& size = Setting::GetInstance().GetWindowSize();

	SetDrawBlendMode(
		DX_BLENDMODE_ALPHA,
		fadeAlpha_);

	DrawBox(
		0,
		0,
		size.width_,
		size.height_,
		GetColor(0, 0, 0),
		TRUE);

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0);
}
