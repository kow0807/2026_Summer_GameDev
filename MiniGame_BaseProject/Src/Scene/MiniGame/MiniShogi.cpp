#include <DxLib.h>
#include <string>
#include <cstring>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"

#include "../../Object/Actor/MiniShogi/Floor.h"
#include "../../Object/Actor/MiniShogi/Shogiban.h"
#include "../../Object/Actor/MiniShogi/Komadai.h"


#include "../../Object/Actor/MiniShogi/Cursor.h"
#include "../../Object/Actor/MiniShogi/Selector.h"
#include "../../Object/Actor/MiniShogi/MiniShogiBoard.h"
#include "../../Object/Actor/MiniShogi/MiniShogiRule.h"
#include "../../Object/Actor/MiniShogi/Hand.h"
#include "../../Object/Actor/MiniShogi/HandActor.h"
#include "../../Object/Actor/MiniShogi/MiniShogiActor.h"
#include "../../Object/Actor/MiniShogi/MiniShogiCpu.h"

#include "../../Application.h"
#include "MiniShogi.h"

MiniShogi::MiniShogi(void)
	:
	isPlayerTurn_(true),
	cpuWaitFrame_(30),
	ruleMessage_(""),
	ruleMessageFrame_(0),
	isGameOver_(false),
	gameOverFrame_(0),
	fontTitle_(-1),
	fontMain_(-1),
	isPlayerWin_(false)
{
}

MiniShogi::~MiniShogi(void)
{
}

void MiniShogi::Init(void)
{
	promotionState_ = PromotionState::NONE;
	promoteSelect_ = true;
	gameOverReason_ = GameOverReason::NONE;

	int screenW;
	int screenH;

	GetWindowSize(
		&screenW,
		&screenH);

	int titleFontSize =
		static_cast<int>(
			32.0f *
			(static_cast<float>(screenH) / 720.0f));

	int mainFontSize =
		static_cast<int>(
			14.0f *
			(static_cast<float>(screenH) / 720.0f));

	if (titleFontSize < 16)
	{
		titleFontSize = 16;
	}

	if (mainFontSize < 10)
	{
		mainFontSize = 10;
	}

	fontTitle_ =
		CreateFontToHandle(
			"游明朝",
			titleFontSize,
			3,
			DX_FONTTYPE_ANTIALIASING);

	fontMain_ =
		CreateFontToHandle(
			"游明朝",
			mainFontSize,
			2,
			DX_FONTTYPE_ANTIALIASING);

	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::MINISHOGI);
	SceneManager::GetInstance().GetCamera()->ChangeShogiTypeCamera(Camera::SHOGI_TYPE::NORMAL);

	floor_ = std::make_unique<Floor>();
	floor_->Init();
	floor_->SetPosition(Floor::DEFAULT_POSITION);

	shogiban_ = std::make_unique<Shogiban>();
	shogiban_->Init();


	pKomadai_ = std::make_unique<Komadai>();
	pKomadai_->Init();
	pKomadai_->SetPosition(Komadai::PLAYER_POSITION);

	eKomadai_ = std::make_unique<Komadai>();
	eKomadai_->Init();
	eKomadai_->SetPosition(Komadai::ENEMY_POSITION);

	cursor_ = std::make_unique<Cursor>();
	cursor_->SetPlayerTurn(isPlayerTurn_);
	cursor_->Init();

	selector_ = std::make_unique<Selector>();
	
	rule_ = std::make_unique <MiniShogiRule>();

	board_ = std::make_unique<MiniShogiBoard>();
	board_->Init();

	player0Hand_ = std::make_unique<Hand>();
	player1Hand_ = std::make_unique<Hand>();

	actor_ = std::make_unique<MiniShogiActor>
		(
		board_.get(),
		player0Hand_.get(),
		player1Hand_.get(),
		cursor_.get(),
		selector_.get(),
		isPlayerTurn_);
	
	actor_->Init();

	cpu_ = std::make_unique<MiniShogiCpu>();

	menuSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MENU_SE).handleId_;
	cancelSe_ = resMng_.Load(ResourceManager::SRC::SELECT_CANCEL_SE).handleId_;
	moveSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MOVE_SE).handleId_;
	decideSEH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_DICIDE_SE).handleId_;

	isPause_ = false;
	pauseScreenHandle_ = MakeScreen(
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		TRUE);
	pauseX_ = -320.0f;
	pauseSelect_ = 0;

	explanationFontHandle_ = CreateFontToHandle(
		"游明朝",
		18,
		3);
}

void MiniShogi::Update(void)
{
	if (PauseUpdate())
	{
		return;
	}

	floor_->Update();
	shogiban_->Update();

	pKomadai_->Update();
	eKomadai_->Update();
	

	if (isGameOver_)
	{
		UpdateGameOver();
		actor_->Update();
		return;
	}

	cursor_->SetHandPieceCount(CursorArea::PLAYER_HAND, actor_->GetPlayerHandActor()->GetPieceCount());
	cursor_->SetHandPieceCount(CursorArea::ENEMY_HAND, actor_->GetEnemyHandActor()->GetPieceCount());

	cursor_->SetPlayerTurn(isPlayerTurn_);

	if (isPlayerTurn_)
	{
		InputUpdate();

		cursor_->Update();

		if (promotionState_ == PromotionState::WAIT_SELECT)
		{
			UpdatePromotion();
		}
		else
		{
			SelectUpdate();
		}
	}
	else
	{
		if (--cpuWaitFrame_ <= 0)
		{
			CpuUpdate();
			cpuWaitFrame_ = 30;
		}
	}

	UpdateCameraState();

	board_->Update();

	actor_->Update();

	CheckGameOver();

	if (ruleMessageFrame_ > 0)
	{
		ruleMessageFrame_--;
	}
}

void MiniShogi::Draw(void)
{
	floor_->Draw();
	shogiban_->Draw();

	pKomadai_->Draw();
	eKomadai_->Draw();
	

	actor_->Draw();

}

void MiniShogi::DrawUI(void)
{
	//----------------------------------
		// 現在の画面サイズを取得
		//----------------------------------
	int screenW;
	int screenH;

	GetWindowSize(
		&screenW,
		&screenH);

	//----------------------------------
	// 解像度変更時にフォントを再作成
	//----------------------------------
	static int lastScreenH = -1;

	if (screenH != lastScreenH)
	{
		if (fontTitle_ != -1)
		{
			DeleteFontToHandle(fontTitle_);
		}

		if (fontMain_ != -1)
		{
			DeleteFontToHandle(fontMain_);
		}

		int titleFontSize =
			static_cast<int>(
				32.0f *
				(static_cast<float>(screenH) / 720.0f));

		int mainFontSize =
			static_cast<int>(
				14.0f *
				(static_cast<float>(screenH) / 720.0f));

		if (titleFontSize < 16)
		{
			titleFontSize = 16;
		}

		if (mainFontSize < 10)
		{
			mainFontSize = 10;
		}

		fontTitle_ =
			CreateFontToHandle(
				"游明朝",
				titleFontSize,
				3,
				DX_FONTTYPE_ANTIALIASING);

		fontMain_ =
			CreateFontToHandle(
				"游明朝",
				mainFontSize,
				2,
				DX_FONTTYPE_ANTIALIASING);

		lastScreenH = screenH;
	}

	//----------------------------------
	// 色
	//----------------------------------
	const unsigned int colorWhite =
		GetColor(240, 230, 215);

	const unsigned int colorGray =
		GetColor(175, 160, 140);

	const unsigned int colorPlayer =
		GetColor(215, 115, 90);

	const unsigned int colorOpponent =
		GetColor(190, 175, 145);

	const unsigned int colorGold =
		GetColor(220, 180, 95);

	const unsigned int colorWarning =
		GetColor(235, 100, 80);

	const unsigned int colorPanel =
		GetColor(40, 28, 20);

	const unsigned int colorLine =
		GetColor(125, 100, 70);

	//----------------------------------
	// 左側UIの座標
	//----------------------------------
	const int panelLeft =
		static_cast<int>(screenW * 0.015f);

	const int panelTop =
		static_cast<int>(screenH * 0.025f);

	const int panelRight =
		static_cast<int>(screenW * 0.245f);

	const int panelBottom =
		static_cast<int>(screenH * 0.70f);

	const int labelX =
		static_cast<int>(screenW * 0.038f);

	const int valueX =
		static_cast<int>(screenW * 0.052f);

	const int lineLeft =
		static_cast<int>(screenW * 0.030f);

	const int lineRight =
		static_cast<int>(screenW * 0.220f);

	const int valueGap =
		static_cast<int>(screenH * 0.040f);

	const int sectionGap =
		static_cast<int>(screenH * 0.125f);

	const int operationGap =
		static_cast<int>(screenH * 0.041f);

	//----------------------------------
	// 左側背景
	//----------------------------------
	SetDrawBlendMode(
		DX_BLENDMODE_ALPHA,
		185);

	DrawBox(
		panelLeft,
		panelTop,
		panelRight,
		panelBottom,
		colorPanel,
		TRUE);

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0);

	//----------------------------------
	// パネル左側の装飾線
	//----------------------------------
	DrawBox(
		panelLeft,
		panelTop,
		panelLeft + static_cast<int>(screenW * 0.004f),
		panelBottom,
		colorGold,
		TRUE);

	//----------------------------------
	// 現在の手番
	//----------------------------------
	const int turnY =
		static_cast<int>(screenH * 0.075f);

	DrawStringToHandle(
		labelX,
		turnY,
		"手番",
		colorGray,
		fontMain_);

	const char* turnText = nullptr;
	unsigned int turnColor = colorWhite;

	if (isPlayerTurn_)
	{
		turnText = "あなた";
		turnColor = colorPlayer;
	}
	else
	{
		turnText = "相手";
		turnColor = colorOpponent;
	}

	DrawStringToHandle(
		valueX,
		turnY + valueGap,
		turnText,
		turnColor,
		fontMain_);

	//----------------------------------
	// 区切り線
	//----------------------------------
	const int turnLineY =
		turnY +
		static_cast<int>(screenH * 0.095f);

	DrawLine(
		lineLeft,
		turnLineY,
		lineRight,
		turnLineY,
		colorLine);

	//----------------------------------
	// 現在の選択場所
	//----------------------------------
	const int selectY =
		turnY + sectionGap;

	DrawStringToHandle(
		labelX,
		selectY,
		"選択場所",
		colorGray,
		fontMain_);

	const char* selectText = "盤面";

	switch (cursor_->GetArea())
	{
	case CursorArea::BOARD:
		selectText = "盤面";
		break;

	case CursorArea::PLAYER_HAND:
		selectText = "自分の持ち駒";
		break;

	case CursorArea::ENEMY_HAND:
		selectText = "相手の持ち駒";
		break;

	default:
		selectText = "";
		break;
	}

	DrawStringToHandle(
		valueX,
		selectY + valueGap,
		selectText,
		colorWhite,
		fontMain_);

	//----------------------------------
	// 区切り線
	//----------------------------------
	const int selectLineY =
		selectY +
		static_cast<int>(screenH * 0.095f);

	DrawLine(
		lineLeft,
		selectLineY,
		lineRight,
		selectLineY,
		colorLine);

	//----------------------------------
	// 対局状態
	//----------------------------------
	const int stateY =
		selectY + sectionGap;

	DrawStringToHandle(
		labelX,
		stateY,
		"対局状況",
		colorGray,
		fontMain_);

	const char* stateText = "対局中";
	unsigned int stateColor = colorWhite;

	if (isGameOver_)
	{
		stateText = "対局終了";
		stateColor = colorGold;
	}
	else if (promotionState_ ==
		PromotionState::WAIT_SELECT)
	{
		stateText = "成りを選択中";
		stateColor = colorGold;
	}
	else if (rule_->IsCheck(
		*board_,
		isPlayerTurn_))
	{
		stateText = "王手";
		stateColor = colorWarning;
	}

	DrawStringToHandle(
		valueX,
		stateY + valueGap,
		stateText,
		stateColor,
		fontMain_);

	//----------------------------------
	// 相手思考中
	//----------------------------------
	if (!isPlayerTurn_ &&
		!isGameOver_)
	{
		const int dotCount =
			(GetNowCount() / 300) % 4;

		std::string thinkingText =
			"相手が考えています";

		for (int i = 0;
			i < dotCount;
			++i)
		{
			thinkingText += "・";
		}

		DrawFormatStringToHandle(
			valueX,
			stateY + valueGap * 2,
			colorGold,
			fontMain_,
			"%s",
			thinkingText.c_str());
	}

	//----------------------------------
	// 操作説明との区切り線
	//----------------------------------
	const int operationLineY =
		static_cast<int>(screenH * 0.505f);

	DrawLine(
		lineLeft,
		operationLineY,
		lineRight,
		operationLineY,
		colorLine);

	//----------------------------------
	// 操作説明
	//----------------------------------
	const int operationY =
		static_cast<int>(screenH * 0.530f);

	DrawStringToHandle(
		labelX,
		operationY,
		"操作方法",
		colorGray,
		fontMain_);

	const int operationTextY =
		operationY + valueGap;

	const bool isPadConnected =
		GetJoypadNum() > 0;

	if (isPadConnected)
	{
		DrawStringToHandle(
			valueX,
			operationTextY,
			"十字ボタン　移動",
			colorWhite,
			fontMain_);

		DrawStringToHandle(
			valueX,
			operationTextY + operationGap,
			"Aボタン　　 決定",
			colorWhite,
			fontMain_);

		DrawStringToHandle(
			valueX,
			operationTextY + operationGap * 2,
			"Bボタン　　 戻る",
			colorWhite,
			fontMain_);

		if (promotionState_ ==
			PromotionState::WAIT_SELECT)
		{
			DrawStringToHandle(
				valueX,
				operationTextY + operationGap * 3,
				"左右　　　　成りを選択",
				colorWhite,
				fontMain_);
		}
	}
	else
	{
		DrawStringToHandle(
			valueX,
			operationTextY,
			"方向キー　　移動",
			colorWhite,
			fontMain_);

		DrawStringToHandle(
			valueX,
			operationTextY + operationGap,
			"Enter　　　 決定",
			colorWhite,
			fontMain_);

		DrawStringToHandle(
			valueX,
			operationTextY + operationGap * 2,
			"BackSpace　 戻る",
			colorWhite,
			fontMain_);

		if (promotionState_ ==
			PromotionState::WAIT_SELECT)
		{
			DrawStringToHandle(
				valueX,
				operationTextY + operationGap * 3,
				"左右キー　　成りを選択",
				colorWhite,
				fontMain_);
		}
	}

	//----------------------------------
	// ルール違反メッセージ
	//----------------------------------
	if (ruleMessageFrame_ > 0 &&
		ruleMessage_ != nullptr &&
		ruleMessage_[0] != '\0')
	{
		const int boxW =
			static_cast<int>(screenW * 0.48f);

		const int boxH =
			static_cast<int>(screenH * 0.07f);

		const int boxX =
			(screenW - boxW) / 2;

		const int boxY =
			static_cast<int>(screenH * 0.88f);

		SetDrawBlendMode(
			DX_BLENDMODE_ALPHA,
			210);

		DrawBox(
			boxX,
			boxY,
			boxX + boxW,
			boxY + boxH,
			GetColor(75, 25, 20),
			TRUE);

		SetDrawBlendMode(
			DX_BLENDMODE_NOBLEND,
			0);

		DrawBox(
			boxX,
			boxY,
			boxX + boxW,
			boxY + boxH,
			colorWarning,
			FALSE);

		const int textWidth =
			GetDrawStringWidthToHandle(
				ruleMessage_,
				static_cast<int>(
					std::strlen(ruleMessage_)),
				fontMain_);

		const int textX =
			boxX +
			(boxW - textWidth) / 2;

		const int textHeight =
			GetFontSizeToHandle(fontMain_);

		const int textY =
			boxY +
			(boxH - textHeight) / 2;

		DrawStringToHandle(
			textX,
			textY,
			ruleMessage_,
			colorWhite,
			fontMain_);
	}

	//----------------------------------
	// ゲームオーバー表示
	//----------------------------------
	if (isGameOver_)
	{
		const int resultWindowW =
			static_cast<int>(screenW * 0.42f);

		const int resultWindowH =
			static_cast<int>(screenH * 0.25f);

		const int resultLeft =
			(screenW - resultWindowW) / 2;

		const int resultTop =
			(screenH - resultWindowH) / 2;

		const int resultRight =
			resultLeft + resultWindowW;

		const int resultBottom =
			resultTop + resultWindowH;

		SetDrawBlendMode(
			DX_BLENDMODE_ALPHA,
			225);

		DrawBox(
			resultLeft,
			resultTop,
			resultRight,
			resultBottom,
			colorPanel,
			TRUE);

		SetDrawBlendMode(
			DX_BLENDMODE_NOBLEND,
			0);

		DrawBox(
			resultLeft,
			resultTop,
			resultRight,
			resultBottom,
			colorGold,
			FALSE);

		const char* resultText =
			isPlayerWin_
			? "勝利"
			: "敗北";

		const char* reasonText = "";

		switch (gameOverReason_)
		{
		case GameOverReason::CHECKMATE:
			reasonText = "詰み";
			break;

		case GameOverReason::NO_LEGAL_MOVE:
			reasonText = "指せる手がありません";
			break;

		case GameOverReason::KING_MISSING:
			reasonText = "王が盤上に存在しません";
			break;

		default:
			reasonText = "";
			break;
		}

		const int resultTextWidth =
			GetDrawStringWidthToHandle(
				resultText,
				static_cast<int>(
					std::strlen(resultText)),
				fontTitle_);

		const int resultTextX =
			resultLeft +
			(resultWindowW - resultTextWidth) / 2;

		const int resultTextY =
			resultTop +
			static_cast<int>(resultWindowH * 0.20f);

		DrawStringToHandle(
			resultTextX,
			resultTextY,
			resultText,
			isPlayerWin_
			? colorGold
			: colorWarning,
			fontTitle_);

		if (reasonText[0] != '\0')
		{
			const int reasonTextWidth =
				GetDrawStringWidthToHandle(
					reasonText,
					static_cast<int>(
						std::strlen(reasonText)),
					fontMain_);

			const int reasonTextX =
				resultLeft +
				(resultWindowW - reasonTextWidth) / 2;

			const int reasonTextY =
				resultTop +
				static_cast<int>(resultWindowH * 0.62f);

			DrawStringToHandle(
				reasonTextX,
				reasonTextY,
				reasonText,
				colorWhite,
				fontMain_);
		}

		return;
	}

	//----------------------------------
	// ポーズ画面
	//----------------------------------
	if (isPause_ ||
		pauseX_ > -320.0f)
	{
		PauseDraw();
	}

	//----------------------------------
	// 成り選択中でなければ終了
	//----------------------------------
	if (promotionState_ !=
		PromotionState::WAIT_SELECT)
	{
		return;
	}

	//----------------------------------
	// 成り選択ウィンドウ
	//----------------------------------
	const int promoteWindowW =
		static_cast<int>(screenW * 0.32f);

	const int promoteWindowH =
		static_cast<int>(screenH * 0.22f);

	const int promoteLeft =
		(screenW - promoteWindowW) / 2;

	const int promoteTop =
		(screenH - promoteWindowH) / 2;

	const int promoteRight =
		promoteLeft + promoteWindowW;

	const int promoteBottom =
		promoteTop + promoteWindowH;

	SetDrawBlendMode(
		DX_BLENDMODE_ALPHA,
		230);

	DrawBox(
		promoteLeft,
		promoteTop,
		promoteRight,
		promoteBottom,
		colorPanel,
		TRUE);

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0);

	DrawBox(
		promoteLeft,
		promoteTop,
		promoteRight,
		promoteBottom,
		colorGold,
		FALSE);

	//----------------------------------
	// 成り確認メッセージ
	//----------------------------------
	const char* promoteMessage =
		"成りますか？";

	const int promoteMessageWidth =
		GetDrawStringWidthToHandle(
			promoteMessage,
			static_cast<int>(
				std::strlen(promoteMessage)),
			fontMain_);

	const int promoteMessageX =
		promoteLeft +
		(promoteWindowW - promoteMessageWidth) / 2;

	const int promoteMessageY =
		promoteTop +
		static_cast<int>(promoteWindowH * 0.18f);

	DrawStringToHandle(
		promoteMessageX,
		promoteMessageY,
		promoteMessage,
		colorWhite,
		fontMain_);

	//----------------------------------
	// 成る・成らないの色
	//----------------------------------
	const unsigned int yesColor =
		promoteSelect_
		? colorGold
		: colorWhite;

	const unsigned int noColor =
		!promoteSelect_
		? colorGold
		: colorWhite;

	const char* yesText = "成る";
	const char* noText = "成らない";

	const int yesTextWidth =
		GetDrawStringWidthToHandle(
			yesText,
			static_cast<int>(
				std::strlen(yesText)),
			fontMain_);

	const int noTextWidth =
		GetDrawStringWidthToHandle(
			noText,
			static_cast<int>(
				std::strlen(noText)),
			fontMain_);

	const int yesCenterX =
		promoteLeft +
		static_cast<int>(promoteWindowW * 0.30f);

	const int noCenterX =
		promoteLeft +
		static_cast<int>(promoteWindowW * 0.70f);

	const int selectionY =
		promoteTop +
		static_cast<int>(promoteWindowH * 0.62f);

	DrawStringToHandle(
		yesCenterX - yesTextWidth / 2,
		selectionY,
		yesText,
		yesColor,
		fontMain_);

	DrawStringToHandle(
		noCenterX - noTextWidth / 2,
		selectionY,
		noText,
		noColor,
		fontMain_);

	//----------------------------------
	// 選択中の項目に下線を表示
	//----------------------------------
	const int underlineY =
		selectionY +
		GetFontSizeToHandle(fontMain_) +
		static_cast<int>(screenH * 0.008f);

	if (promoteSelect_)
	{
		DrawLine(
			yesCenterX - yesTextWidth / 2,
			underlineY,
			yesCenterX + yesTextWidth / 2,
			underlineY,
			colorGold);
	}
	else
	{
		DrawLine(
			noCenterX - noTextWidth / 2,
			underlineY,
			noCenterX + noTextWidth / 2,
			underlineY,
			colorGold);
	}
}

void MiniShogi::Reset(void)
{
	Setting::GetInstance().SetFullScreen(FALSE);
}

void MiniShogi::InputUpdate(void)
{
	auto& ins = InputManager::GetInstance();

	if (promotionState_ == PromotionState::WAIT_SELECT)
	{
		return;
	}

	if (ins.IsTrgUp(KEY_INPUT_UP) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP)) cursor_->MoveUp();
	if (ins.IsTrgUp(KEY_INPUT_DOWN) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN)) cursor_->MoveDown();
	if (ins.IsTrgUp(KEY_INPUT_LEFT) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_LEFT)) cursor_->MoveLeft();
	if (ins.IsTrgUp(KEY_INPUT_RIGHT) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_RIGHT)) cursor_->MoveRight();
}

void MiniShogi::UpdateCameraState(void)
{
	auto& camera = *SceneManager::GetInstance().GetCamera();

	Camera::SHOGI_TYPE nextType = Camera::SHOGI_TYPE::NORMAL;

	switch (cursor_->GetArea())
	{
	case CursorArea::BOARD:
		nextType = Camera::SHOGI_TYPE::NORMAL;
		break;

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		nextType = Camera::SHOGI_TYPE::HAND_SELECT;
		break;
	default:
		nextType = Camera::SHOGI_TYPE::NORMAL;
		break;
	}

	if (camera.GetShogiType() != nextType)
	{
		camera.ChangeShogiTypeCamera(nextType);
	}
}

void MiniShogi::SelectUpdate(void)
{
	auto& ins =
		InputManager::GetInstance();

	//----------------------------------
	// 選択解除
	// BackSpace または パッドB
	//----------------------------------
	const bool isCancel =
		ins.IsTrgUp(KEY_INPUT_BACK) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::RIGHT);

	if (isCancel)
	{
		CancelSelect();
		return;
	}

	//----------------------------------
	// 決定
	// Enter または パッドA
	//----------------------------------
	const bool isDecide =
		ins.IsTrgUp(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);

	if (!isDecide)
	{
		return;
	}

	//----------------------------------
	// 駒をまだ選択していない
	//----------------------------------
	if (!selector_->IsSelecting())
	{
		switch (cursor_->GetArea())
		{
		case CursorArea::BOARD:
			SelectBoardPiece();
			break;

		case CursorArea::PLAYER_HAND:
		case CursorArea::ENEMY_HAND:
			SelectHandPiece();
			break;

		default:
			break;
		}

		return;
	}

	//----------------------------------
	// 駒を選択済み
	//----------------------------------
	switch (selector_->GetSelectArea())
	{
	case CursorArea::BOARD:
		MoveBoardPiece();
		break;

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		DropHandPiece();
		break;

	default:
		CancelSelect();
		break;
	}
}

void MiniShogi::UpdatePromotion(void)
{
	auto& ins =
		InputManager::GetInstance();

	//----------------------------------
	// はい・いいえの切り替え
	//----------------------------------
	const bool isMoveSelect =
		ins.IsTrgUp(KEY_INPUT_LEFT) ||
		ins.IsTrgUp(KEY_INPUT_RIGHT) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DPAD_LEFT) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DPAD_RIGHT);

	if (isMoveSelect)
	{
		promoteSelect_ =
			!promoteSelect_;
	}

	//----------------------------------
	// 決定
	// Enter または パッドA
	//----------------------------------
	const bool isDecide =
		ins.IsTrgUp(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);

	if (!isDecide)
	{
		return;
	}

	//----------------------------------
	// 「はい」なら成る
	//----------------------------------
	if (promoteSelect_)
	{
		rule_->Promote(
			pendingMovePiece_);
	}

	//----------------------------------
	// 移動を確定
	//----------------------------------
	board_->SetPiece(
		pendingToX_,
		pendingToY_,
		pendingMovePiece_);

	board_->RemovePiece(
		pendingFromX_,
		pendingFromY_);

	selector_->Select(false);

	promotionState_ =
		PromotionState::NONE;

	isPlayerTurn_ =
		!isPlayerTurn_;

	CheckGameOver();
}

void MiniShogi::SelectBoardPiece(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	if (!board_->IsExistPiece(x, y))
	{
		return;
	}

	if (!rule_->CanSelectPiece(*board_, x, y, isPlayerTurn_)) return;

	selector_->Select(true);

	selector_->SetSelectPositon(CursorArea::BOARD, x, y, 0);

	selector_->SetSelectPieceType(board_->GetPiece(x, y).type_);

	selector_->SetMoveList(rule_->GetLegalMoveList(*board_, x, y));
}

void MiniShogi::MoveBoardPiece(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	if (!selector_->IsMovePosition(x, y))
	{
		selector_->Select(false);
		return;
	}
	
	ExecuteMove(
		selector_->GetSelectX(),
		selector_->GetSelectY(),
		x,
		y);

	selector_->Select(false);
}

void MiniShogi::SelectHandPiece(void)
{
	Hand& hand = GetCurrentHand();

	if (!rule_->CanSelectHandPiece(hand, cursor_->GetHandIndex()))
	{
		return;
	}

	selector_->Select(true);

	selector_->SetSelectPositon(
		cursor_->GetArea(),
		0,
		0,
		cursor_->GetHandIndex());

	selector_->SetSelectPieceType(
		hand.GetPiece(cursor_->GetHandIndex()).type_);

	PieceType pieceType = hand.GetPiece(cursor_->GetHandIndex()).type_;

	// 打てる場所のリストを取得
	selector_->SetMoveList(rule_->GetDropList(*board_, pieceType, isPlayerTurn_));

	// 盤面へカーソルを戻す
	cursor_->ChangeArea(CursorArea::BOARD);

	cursor_->SetBoardPosition(2, 2);
}

void MiniShogi::DropHandPiece(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	if (!selector_->IsMovePosition(x, y))
	{
		DropError error;

		rule_->CanDrop(
			*board_,
			selector_->GetSelectPieceType(),
			x,
			y,
			isPlayerTurn_,
			error);

		ruleMessage_ = rule_->GetDropErrorMessage(error);
		ruleMessageFrame_ = 90;

		selector_->Select(false);
		return;
	}


	ExecuteDrop(
		selector_->GetSelectPieceType(),
		x,
		y);

	selector_->Select(false);
}

bool MiniShogi::MovePiece(int fromX, int fromY, int toX, int toY)
{
	Piece movePiece = board_->GetPiece(fromX, fromY);

	// 駒取得
	if (board_->IsExistPiece(toX, toY))
	{
		Piece capturePiece =
			board_->GetPiece(toX, toY);

		// 王は取らない
		if (capturePiece.type_ == PieceType::OU)
		{
			return false;
		}

		capturePiece.isPlayer_ = isPlayerTurn_;
		capturePiece.isPromote_ = false;

		if (isPlayerTurn_)
		{
			player0Hand_->AddPiece(capturePiece.type_);
		}
		else
		{
			player1Hand_->AddPiece(capturePiece.type_);
		}
	}

	// 強制成り
	if (rule_->MustPromote(movePiece, toY))
	{
		rule_->Promote(movePiece);
	}
	// 任意成り
	else if (rule_->CanPromote(movePiece, fromY, toY))
	{
		pendingMovePiece_ = movePiece;

		pendingFromX_ = fromX;
		pendingFromY_ = fromY;

		pendingToX_ = toX;
		pendingToY_ = toY;

		promotionState_ = PromotionState::WAIT_SELECT;

		promoteSelect_ = true;

		return false;
	}

	board_->SetPiece(toX, toY, movePiece);

	board_->RemovePiece(fromX, fromY);

	return true;
}

Hand& MiniShogi::GetCurrentHand(void)
{
	if (isPlayerTurn_)
	{
		return *player0Hand_;
	}

	return *player1Hand_;
}

const Hand& MiniShogi::GetCurrentHand(void) const
{
	if (isPlayerTurn_)
	{
		return *player0Hand_;
	}

	return *player1Hand_;
}

bool MiniShogi::ExecuteMove(int fromX, int fromY, int toX, int toY)
{
	if (!rule_->IsLegalMove(
		*board_,
		fromX,
		fromY,
		toX,
		toY))
	{
		ruleMessage_ =
			"王手を放置する手は指せません";

		ruleMessageFrame_ = 90;

		return false;
	}

	if (!MovePiece(
		fromX,
		fromY,
		toX,
		toY))
	{
		return false;
	}

	isPlayerTurn_ = !isPlayerTurn_;

	CheckGameOver();

	return true;
}

bool MiniShogi::ExecuteDrop(PieceType pieceType, int toX, int toY)
{
	DropError error;

	if (!rule_->CanDrop(
		*board_,
		pieceType,
		toX,
		toY,
		isPlayerTurn_,
		error))
	{
		ruleMessage_ =
			rule_->GetDropErrorMessage(error);

		ruleMessageFrame_ = 90;

		return false;
	}

	Hand& hand = GetCurrentHand();

	if (!hand.HasPiece(pieceType))
	{
		return false;
	}

	//----------------------------------
	// 配置後も自玉が王手か確認
	//----------------------------------
	MiniShogiBoard testBoard = *board_;

	Piece testPiece
	{
		pieceType,
		isPlayerTurn_,
		false
	};

	testBoard.SetPiece(
		toX,
		toY,
		testPiece);

	if (rule_->IsCheck(
		testBoard,
		isPlayerTurn_))
	{
		ruleMessage_ =
			"王手を解除できない場所には打てません";

		ruleMessageFrame_ = 90;

		return false;
	}

	board_->SetPiece(
		toX,
		toY,
		testPiece);

	hand.RemovePiece(pieceType);

	isPlayerTurn_ = !isPlayerTurn_;

	CheckGameOver();

	return true;
}

void MiniShogi::CheckGameOver(void)
{
	if (isGameOver_)
	{
		return;
	}

	//----------------------------------
	// 王が盤面に存在するか
	//----------------------------------
	const bool playerKingExist =
		rule_->IsKingExist(
			*board_,
			true);

	const bool cpuKingExist =
		rule_->IsKingExist(
			*board_,
			false);

	if (!playerKingExist)
	{
		isGameOver_ = true;
		isPlayerWin_ = false;

		gameOverReason_ =
			GameOverReason::KING_MISSING;

		gameOverFrame_ = 180;
		return;
	}

	if (!cpuKingExist)
	{
		isGameOver_ = true;
		isPlayerWin_ = true;

		gameOverReason_ =
			GameOverReason::KING_MISSING;

		gameOverFrame_ = 180;
		return;
	}

	//----------------------------------
	// 現在の手番側
	//----------------------------------
	const bool currentSide =
		isPlayerTurn_;

	const Hand& currentHand =
		currentSide
		? *player0Hand_
		: *player1Hand_;

	const bool isCheck =
		rule_->IsCheck(
			*board_,
			currentSide);

	const bool hasLegalMove =
		rule_->HasAnyLegalMove(
			*board_,
			currentHand,
			currentSide);

	//----------------------------------
	// 指せる手があるなら続行
	//----------------------------------
	if (hasLegalMove)
	{
		return;
	}

	//----------------------------------
	// 指せる手がないので終局
	//----------------------------------
	isGameOver_ = true;

	// 現在CPU手番ならプレイヤー勝利
	// 現在プレイヤー手番ならCPU勝利
	isPlayerWin_ = !currentSide;

	if (isCheck)
	{
		gameOverReason_ =
			GameOverReason::CHECKMATE;
	}
	else
	{
		gameOverReason_ =
			GameOverReason::NO_LEGAL_MOVE;
	}

	// 60FPSで約3秒
	gameOverFrame_ = 180;

}

void MiniShogi::UpdateGameOver(void)
{
	if (!isGameOver_)
	{
		return;
	}

	if (gameOverFrame_ > 0)
	{
		gameOverFrame_--;
		return;
	}

	isReturn_ = true;
}

void MiniShogi::CancelSelect(void)
{
	if (!selector_->IsSelecting())
	{
		return;
	}

	selector_->Select(false);

	// 持ち駒選択後に盤面へ移動していた場合も、
	// カーソル位置はそのままで選択だけ解除する
	ruleMessage_ = "選択を解除しました";
	ruleMessageFrame_ = 60;
}

void MiniShogi::CpuUpdate(void)
{
	if (isGameOver_)
	{
		return;
	}

	CpuMove move =
		cpu_->Think(
			*board_,
			*player1Hand_,
			*rule_);

	//----------------------------------
	// CPUに指せる手がない
	//----------------------------------
	if (move.pieceType == PieceType::NONE)
	{
		CheckGameOver();

		//----------------------------------
		// Rule側では合法手があるのに、
		// CPUが手を生成できなかった場合
		//----------------------------------
		if (!isGameOver_)
		{
			ruleMessage_ =
				"CPUの手生成に失敗しました";

			ruleMessageFrame_ = 180;
		}

		return;
	}

	//----------------------------------
	// 持ち駒を打つ
	//----------------------------------
	if (move.isDrop)
	{
		const bool result =
			ExecuteDrop(
				move.pieceType,
				move.toX,
				move.toY);

		if (!result)
		{
			ruleMessage_ =
				"CPUが不正な駒打ちを選びました";

			ruleMessageFrame_ = 180;
		}

		return;
	}

	//----------------------------------
	// 盤上の駒を動かす
	//----------------------------------
	const bool result =
		ExecuteCpuMove(
			move.fromX,
			move.fromY,
			move.toX,
			move.toY,
			move.isPromote);

	if (!result)
	{
		ruleMessage_ =
			"CPUが不正な移動を選びました";

		ruleMessageFrame_ = 180;
	}
}

bool MiniShogi::ExecuteCpuMove(int fromX, int fromY, int toX, int toY, bool isPromote)
{

	MiniShogiBoard testBoard = *board_;

	Piece movePiece =
		testBoard.GetPiece(fromX, fromY);

	if (isPromote || rule_->MustPromote(movePiece, toY))
	{
		rule_->Promote(movePiece);
	}

	testBoard.SetPiece(toX, toY, movePiece);
	testBoard.RemovePiece(fromX, fromY);

	if (rule_->IsCheck(testBoard, isPlayerTurn_))
	{
		return false;
	}

	Piece realMovePiece =
		board_->GetPiece(fromX, fromY);

	if (board_->IsExistPiece(toX, toY))
	{
		Piece capturePiece =
			board_->GetPiece(toX, toY);

		// 王を取る手は実行しない
		if (capturePiece.type_ == PieceType::OU)
		{
			return false;
		}

		capturePiece.isPlayer_ =
			isPlayerTurn_;

		capturePiece.isPromote_ = false;

		player1Hand_->AddPiece(
			capturePiece.type_);
	}

	if (isPromote || rule_->MustPromote(realMovePiece, toY))
	{
		rule_->Promote(realMovePiece);
	}

	board_->SetPiece(toX, toY, realMovePiece);
	board_->RemovePiece(fromX, fromY);

	isPlayerTurn_ = !isPlayerTurn_;

	CheckGameOver();

	return true;
}

bool MiniShogi::PauseUpdate(void)
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
		}
		else
		{
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
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

void MiniShogi::PauseDraw(void)
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