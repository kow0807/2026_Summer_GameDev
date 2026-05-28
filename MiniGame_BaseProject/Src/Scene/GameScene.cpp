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
#include "../Scene/MiniGame/Quoridor.h"
#include "GameScene.h"

GameScene::GameScene(void)
	: 
	miniState_(MINI_STATE::FIRST_PRESS),
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

	explanationFontHandle_ = CreateFontToHandle(
		"游明朝",
		18,
		3);

	firstPressExplanationImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_EXPLANATION).handleId_;
	quizExplanationImg_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_EXPLANATION).handleId_;
	buttonMashExplanationImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_EXPLANATION).handleId_;
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
		if(gameBase_)
		{
			gameBase_->DrawUI();
		}
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
			//　生成と初期化
			CreateMiniGame();
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

	if (gameBase_->GetIsReturn())
	{
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

void GameScene::DrawGame()
{
	if (!gameBase_)
	{
		return;
	}

	gameBase_->Draw();
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
		"オセロ(開発中のため\n選択しないでください)",
		"連打対決",
		"フラッシュ暗算(開発中のため\n選択しないでください)",
		"コリドール",
		"ウサギと猟犬(開発中のため\n選択しないでください)",
		"5五将棋(開発中のため\n選択しないでください)"
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
	switch (miniState_)
	{
	case MINI_STATE::FIRST_PRESS:
		ExplanationFirstPressDrawUI();
		break;
	case MINI_STATE::QUIZ:
		ExplanationQuizDrawUI();
		break;
	case MINI_STATE::REVERSI:
		break;
	case MINI_STATE::BUTTON_MASH:
		ExplanationButtonMashDrawUI();
		break;
	case MINI_STATE::FLASH_CALC:
		break;
	case MINI_STATE::QUORIDOR:
		ExplanationQuoridorDrawUI();
		return;
		break;
	case MINI_STATE::HARE_AND_HOUNDS:
		break;
	case MINI_STATE::MINI_SHOGI:
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

	DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

	DrawStringToHandle(textX, textY + 90, "・合図が表示された瞬間に", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX + 25, textY + 130, "Spaceキーを押してください", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX, textY + 210, "・2本先取で勝利", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX, textY + 290, "・フェイクに騙されないように", GetColor(255, 220, 120), explanationFontHandle_);

	DrawStringToHandle(textX + 25, textY + 330, "気を付けましょう", GetColor(255, 220, 120), explanationFontHandle_);

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, strlen(startText), explanationFontHandle_);

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

	int yesTextW = GetDrawStringWidthToHandle(yesText, strlen(yesText), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, strlen(noText), explanationFontHandle_);

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

	DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

	DrawStringToHandle(textX, textY + 90, "・クイズに10問中8問", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX + 25, textY + 130, "正解でクリア", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX, textY + 210, "・矢印キーで回答を選択", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX, textY + 290, "・Enterキーで決定", GetColor(220, 220, 220), explanationFontHandle_);

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, strlen(startText), explanationFontHandle_);

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

	int yesTextW = GetDrawStringWidthToHandle(yesText, strlen(yesText), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, strlen(noText), explanationFontHandle_);

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

	DrawLine(textX, textY + 40, textX + 320, textY + 40, GetColor(0, 180, 255));

	DrawStringToHandle(textX, textY + 90, "・合図が表示された後", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX + 25, textY + 130, "Spaceキーを連打してください", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX, textY + 210, "・連打の優勢状況によって", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX + 25, textY + 250, "画面の色が変化します", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX, textY + 330, "・青で埋め尽くすと1本獲得し", GetColor(220, 220, 220), explanationFontHandle_);

	DrawStringToHandle(textX + 25, textY + 370, "2本先取で勝利", GetColor(220, 220, 220), explanationFontHandle_);

	// 下部 : 開始確認
	DrawLine(panelX + 40, panelY + 540, panelX + panelW - 40, panelY + 540, GetColor(0, 180, 255));


	// タイトル文字を中央配置
	const char* startText = "ゲームを開始しますか？";

	int startTextW = GetDrawStringWidthToHandle(startText, strlen(startText), explanationFontHandle_);

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

	int yesTextW = GetDrawStringWidthToHandle(yesText, strlen(yesText), explanationFontHandle_);

	DrawStringToHandle(yesX + buttonW / 2 - yesTextW / 2, buttonY + 12, yesText, yesColor, explanationFontHandle_);


	// いいえボタン
	int noX = yesX + buttonW + buttonGap;

	DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(25, 35, 55), true);

	if (!isYes_)
	{
		DrawBox(noX, buttonY, noX + buttonW, buttonY + buttonH, GetColor(255, 255, 0), false);
	}

	const char* noText = "いいえ";

	int noTextW = GetDrawStringWidthToHandle(noText, strlen(noText), explanationFontHandle_);

	DrawStringToHandle(noX + buttonW / 2 - noTextW / 2, buttonY + 12, noText, noColor, explanationFontHandle_);

}

void GameScene::ExplanationQuoridorDrawUI(void)
{
	int index = static_cast<int>(miniState_);

	// 🎨 画面解像度に応じた比率（スケール）の計算
	const auto& windowSize = Setting::GetInstance().GetWindowSize();
	float scaleX = static_cast<float>(windowSize.width_) / 1280.0f;
	float scaleY = static_cast<float>(windowSize.height_) / 720.0f;

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

void GameScene::CreateMiniGame(void)
{
	// gameBase_のポインタ生成ができていないところを選択した場合
	// 中身がないので例外スローになる。追加する際はここに生成コードを追加せよ
	
	switch (miniState_)
	{
	case MINI_STATE::FIRST_PRESS:
		gameBase_ = std::make_unique<FirstPressGame>();
		gameBase_->Init();
		break;
	case MINI_STATE::QUIZ:
		gameBase_ = std::make_unique<QuizGame>();
		gameBase_->Init();
		break;
	case MINI_STATE::REVERSI:
		break;
	case MINI_STATE::BUTTON_MASH:
		gameBase_ = std::make_unique<ButtonMashGame>();
		gameBase_->Init();
		break;
	case MINI_STATE::FLASH_CALC:
		break;
	case MINI_STATE::QUORIDOR:
		gameBase_ = std::make_unique<Quoridor>();
		break;
	case MINI_STATE::HARE_AND_HOUNDS:
		break;
	case MINI_STATE::MINI_SHOGI:
		break;
	}

	gameBase_->Init();
}
