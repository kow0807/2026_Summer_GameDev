#include <DxLib.h>
#include <string>
#include <cstring>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"

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

	shogiban_ = std::make_unique<Shogiban>();
	shogiban_->Init();

	komadai_ = std::make_unique<Komadai>();
	komadai_->Init();

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
}

void MiniShogi::Update(void)
{

	shogiban_->Update();
	komadai_->Update();

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

	//shogiban_->Draw();
	komadai_->Draw();
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
	static int lastScreenH = screenH;

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
	GetColor(240, 230, 220);

	const unsigned int colorGray =
	GetColor(150, 140, 130);

	const unsigned int colorPlayer =
	GetColor(200, 80, 80);

	const unsigned int colorCpu =
	GetColor(80, 130, 230);

	const unsigned int colorYellow =
	GetColor(240, 200, 80);

	const unsigned int colorWarning =
	GetColor(255, 120, 100);

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
	static_cast<int>(screenH * 0.7f);

	const int leftX =
	static_cast<int>(screenW * 0.04f);

	const int leftX1 =
		static_cast<int>(screenW * 0.45f);


	const int lineGap =
	static_cast<int>(screenH * 0.04f);

	const int itemGap =
	static_cast<int>(screenH * 0.042f);

	//----------------------------------
	// 左側背景
	//----------------------------------
	SetDrawBlendMode(
		DX_BLENDMODE_ALPHA,
		170);

	DrawBox(
		panelLeft,
		panelTop,
		panelRight,
		panelBottom,
		GetColor(25, 20, 15),
		TRUE);

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0);

	//----------------------------------
	// タイトル
	//----------------------------------
	DrawStringToHandle(
		leftX - static_cast<int>(screenW * 0.015f),
		static_cast<int>(screenH * 0.05f),
		"MINI SHOGI",
		colorWhite,
		fontTitle_);

	//----------------------------------
	// 現在の手番
	//----------------------------------
	const int turnY =
	static_cast<int>(screenH * 0.17f);

	DrawStringToHandle(
		leftX,
		turnY,
		"TURN",
		colorGray,
		fontMain_);

	if (isPlayerTurn_)
	{
		DrawStringToHandle(
			leftX,
			turnY + lineGap,
			"あなたの手番",
			colorPlayer,
			fontMain_);
	}
	else
	{
		DrawStringToHandle(
			leftX,
			turnY + lineGap,
			"CPUの手番",
			colorCpu,
			fontMain_);
	}

	//----------------------------------
	// 現在の選択場所
	//----------------------------------
	const int selectY =
	static_cast<int>(screenH * 0.29f);

	DrawStringToHandle(
		leftX,
		selectY,
		"SELECT",
		colorGray,
		fontMain_);

	const char* selectText = "盤面";

	switch (cursor_->GetArea())
	{
	case CursorArea::BOARD:
		selectText = "盤面";
		break;

	case CursorArea::PLAYER_HAND:
		selectText = "あなたの持ち駒";
		break;

	case CursorArea::ENEMY_HAND:
		selectText = "CPUの持ち駒";
		break;

	default:
		selectText = "";
		break;
	}

	DrawStringToHandle(
		leftX,
		selectY + lineGap,
		selectText,
		colorWhite,
		fontMain_);

	//----------------------------------
	// 状態表示
	//----------------------------------
	const int stateY =
	static_cast<int>(screenH * 0.40f);

	DrawStringToHandle(
		leftX,
		stateY,
		"STATUS",
		colorGray,
		fontMain_);

	if (isGameOver_)
	{
		DrawStringToHandle(
			leftX,
			stateY + lineGap,
			"対局終了",
			colorYellow,
			fontMain_);
	}
	else if (promotionState_ ==
		PromotionState::WAIT_SELECT)
	{
		DrawStringToHandle(
			leftX,
			stateY + lineGap,
			"成りを選択中",
			colorYellow,
			fontMain_);
	}
	else if (rule_->IsCheck(
		*board_,
		isPlayerTurn_))
	{
		DrawStringToHandle(
			leftX,
			stateY + lineGap,
			"王手されています",
			colorWarning,
			fontMain_);
	}
	else
	{
		DrawStringToHandle(
			leftX,
			stateY + lineGap,
			"対局中",
			colorWhite,
			fontMain_);
	}

	//----------------------------------
	// CPU思考中
	//----------------------------------
	if (!isPlayerTurn_ &&
		!isGameOver_)
	{
		const int dotCount =
			(GetNowCount() / 300) % 4;

		std::string thinkingText =
			"CPU思考中";

		for (int i = 0;
			i < dotCount;
			i++)
		{
			thinkingText += ".";
		}

		DrawFormatStringToHandle(
			leftX,
			stateY + lineGap * 2,
			colorYellow,
			fontMain_,
			"%s",
			thinkingText.c_str());
	}

	//----------------------------------
	// 操作説明
	//----------------------------------
	const int menuY =
	static_cast<int>(screenH * 0.56f);

	DrawStringToHandle(
		leftX,
		menuY - itemGap,
		"CONTROL",
		colorGray,
		fontMain_);

	const bool isPadConnected =
	GetJoypadNum() > 0;

	if (isPadConnected)
	{
		DrawStringToHandle(
			leftX,
			menuY,
			"移動      ：十字ボタン",
			colorGray,
			fontMain_);

		DrawStringToHandle(
			leftX,
			menuY + itemGap,
			"決定      ：A",
			colorGray,
			fontMain_);

		DrawStringToHandle(
			leftX,
			menuY + itemGap * 2,
			"選択解除  ：B",
			colorGray,
			fontMain_);

		if (promotionState_ ==
			PromotionState::WAIT_SELECT)
		{
			DrawStringToHandle(
				leftX,
				menuY + itemGap * 3,
				"成り選択  ：左右",
				colorGray,
				fontMain_);
		}
	}
	else
	{
		DrawStringToHandle(
			leftX,
			menuY,
			"移動      ：方向キー",
			colorGray,
			fontMain_);

		DrawStringToHandle(
			leftX,
			menuY + itemGap,
			"決定      ：ENTER",
			colorGray,
			fontMain_);

		DrawStringToHandle(
			leftX,
			menuY + itemGap * 2,
			"選択解除  ：BackSpace",
			colorGray,
			fontMain_);

		if (promotionState_ ==
			PromotionState::WAIT_SELECT)
		{
			DrawStringToHandle(
				leftX,
				menuY + itemGap * 3,
				"成り選択  ：左右キー",
				colorGray,
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
			190);

		DrawBox(
			boxX,
			boxY,
			boxX + boxW,
			boxY + boxH,
			GetColor(80, 20, 20),
			TRUE);

		SetDrawBlendMode(
			DX_BLENDMODE_NOBLEND,
			0);

		const int textWidth =
			GetDrawStringWidthToHandle(
				ruleMessage_,
				static_cast<int>(
					std::strlen(ruleMessage_)),
				fontMain_);

		const int textX =
			boxX +
			(boxW - textWidth) / 2;

		const int textY =
			boxY +
			static_cast<int>(boxH * 0.28f);

		DrawStringToHandle(
			textX,
			textY,
			ruleMessage_,
			colorWarning,
			fontMain_);
	}

	//----------------------------------
	// ゲームオーバー表示
	//----------------------------------
	if (isGameOver_)
	{
		const int resultTop =
			static_cast<int>(screenH * 0.47f);

		const char* resultText =
			isPlayerWin_
			? "あなたの勝ち"
			: "CPUの勝ち";

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
			break;
		}

		DrawStringToHandle(
			leftX1,
			resultTop,
			resultText,
			colorYellow,
			fontTitle_);

		DrawStringToHandle(
			leftX1,
			resultTop +
			static_cast<int>(screenH * 0.065f),
			reasonText,
			colorWhite,
			fontMain_);

		return;
	}

	//----------------------------------
	// 成り選択ウィンドウ
	//----------------------------------
	if (promotionState_ !=
		PromotionState::WAIT_SELECT)
	{
		return;
	}

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
		220);

	DrawBox(
		promoteLeft,
		promoteTop,
		promoteRight,
		promoteBottom,
		GetColor(40, 40, 40),
		TRUE);

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0);

	DrawBox(
		promoteLeft,
		promoteTop,
		promoteRight,
		promoteBottom,
		colorWhite,
		FALSE);

	DrawStringToHandle(
		promoteLeft +
		static_cast<int>(promoteWindowW * 0.29f),
		promoteTop +
		static_cast<int>(promoteWindowH * 0.16f),
		"成りますか？",
		colorWhite,
		fontMain_);

	const unsigned int yesColor =
	promoteSelect_
	? colorYellow
		: colorWhite;

	const unsigned int noColor =
	!promoteSelect_
	? colorYellow
		: colorWhite;

	DrawStringToHandle(
		promoteLeft +
		static_cast<int>(promoteWindowW * 0.22f),
		promoteTop +
		static_cast<int>(promoteWindowH * 0.62f),
		"はい",
		yesColor,
		fontMain_);

	DrawStringToHandle(
		promoteLeft +
		static_cast<int>(promoteWindowW * 0.63f),
		promoteTop +
		static_cast<int>(promoteWindowH * 0.62f),
		"いいえ",
		noColor,
		fontMain_);
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
