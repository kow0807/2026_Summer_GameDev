#include  <DxLib.h>
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
	isReturn_(false),
	gameOverFrame_(0),
	isPlayerWin_(false)
{
}

MiniShogi::~MiniShogi(void)
{
}

void MiniShogi::Init(void)
{
	//SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FREE);

	promotionState_ = PromotionState::NONE;
	promoteSelect_ = true;
	gameOverReason_ = GameOverReason::NONE;

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
	// ルールエラー表示
	//----------------------------------
	if (ruleMessageFrame_ > 0)
	{
		DrawString(
			350,
			560,
			ruleMessage_,
			GetColor(255, 80, 80));
	}

	//----------------------------------
	// 勝敗表示
	//----------------------------------
	if (isGameOver_)
	{
		const char* resultText =
			isPlayerWin_
			? "あなたの勝ちです"
			: "CPUの勝ちです";

		const char* reasonText = "";

		switch (gameOverReason_)
		{
		case GameOverReason::CHECKMATE:
			reasonText = "詰み";
			break;

		case GameOverReason::KING_MISSING:
			reasonText = "王が取られました";
			break;

		default:
			reasonText = "";
			break;
		}

		constexpr int WINDOW_W = 400;
		constexpr int WINDOW_H = 160;
		constexpr int SCREEN_W = 1024;
		constexpr int SCREEN_H = 640;

		const int left =
			(SCREEN_W - WINDOW_W) / 2;

		const int top =
			(SCREEN_H - WINDOW_H) / 2;

		const int right =
			left + WINDOW_W;

		const int bottom =
			top + WINDOW_H;

		DrawBox(
			left,
			top,
			right,
			bottom,
			GetColor(40, 40, 40),
			TRUE);

		DrawBox(
			left,
			top,
			right,
			bottom,
			GetColor(255, 255, 255),
			FALSE);

		DrawString(
			left + 120,
			top + 45,
			resultText,
			GetColor(255, 255, 0));

		DrawString(
			left + 145,
			top + 95,
			reasonText,
			GetColor(255, 255, 255));

		return;
	}

	//----------------------------------
	// 成り選択中でなければ終了
	//----------------------------------
	if (promotionState_ != PromotionState::WAIT_SELECT)
	{
		return;
	}

	constexpr int WINDOW_W = 340;
	constexpr int WINDOW_H = 160;

	constexpr int SCREEN_W = 1024;
	constexpr int SCREEN_H = 640;

	const int left =
		(SCREEN_W - WINDOW_W) / 2;

	const int top =
		(SCREEN_H - WINDOW_H) / 2;

	const int right =
		left + WINDOW_W;

	const int bottom =
		top + WINDOW_H;

	DrawBox(
		left,
		top,
		right,
		bottom,
		GetColor(40, 40, 40),
		TRUE);

	DrawBox(
		left,
		top,
		right,
		bottom,
		GetColor(255, 255, 255),
		FALSE);

	DrawString(
		left + 95,
		top + 25,
		"成りますか？",
		GetColor(255, 255, 255));

	DrawString(
		left + 70,
		top + 90,
		"はい",
		promoteSelect_
		? GetColor(255, 255, 0)
		: GetColor(255, 255, 255));

	DrawString(
		left + 190,
		top + 90,
		"いいえ",
		!promoteSelect_
		? GetColor(255, 255, 0)
		: GetColor(255, 255, 255));
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

	if (ins.IsTrgUp(KEY_INPUT_UP)) cursor_->MoveUp();
	if (ins.IsTrgUp(KEY_INPUT_DOWN)) cursor_->MoveDown();
	if (ins.IsTrgUp(KEY_INPUT_LEFT)) cursor_->MoveLeft();
	if (ins.IsTrgUp(KEY_INPUT_RIGHT)) cursor_->MoveRight();
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
	auto& ins = InputManager::GetInstance();

	if (!ins.IsTrgUp(KEY_INPUT_RETURN)) return;

	// 未選択時
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
		}

		return;
	}

	// 選択済み
	switch (selector_->GetSelectArea())
	{
	case CursorArea::BOARD:
		MoveBoardPiece();
		break;

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		DropHandPiece();
		break;
	}
}

void MiniShogi::UpdatePromotion(void)
{
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgUp(KEY_INPUT_LEFT) ||
		ins.IsTrgUp(KEY_INPUT_RIGHT))
	{
		promoteSelect_ = !promoteSelect_;
	}

	if (!ins.IsTrgUp(KEY_INPUT_RETURN))
	{
		return;
	}

	if (promoteSelect_)
	{
		rule_->Promote(pendingMovePiece_);
	}

	board_->SetPiece(
		pendingToX_,
		pendingToY_,
		pendingMovePiece_);

	board_->RemovePiece(
		pendingFromX_,
		pendingFromY_);

	selector_->Select(false);

	promotionState_ = PromotionState::NONE;

	isPlayerTurn_ = !isPlayerTurn_;

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
	// 王の消失は異常時の保険
	//----------------------------------
	if (!rule_->IsKingExist(
		*board_,
		true))
	{
		isGameOver_ = true;
		isPlayerWin_ = false;
		gameOverReason_ =
			GameOverReason::KING_MISSING;
		gameOverFrame_ = 180;
		return;
	}

	if (!rule_->IsKingExist(
		*board_,
		false))
	{
		isGameOver_ = true;
		isPlayerWin_ = true;
		gameOverReason_ =
			GameOverReason::KING_MISSING;
		gameOverFrame_ = 180;
		return;
	}

	//----------------------------------
	// 現在手番側が詰んでいるか
	//----------------------------------
	const Hand& currentHand =
		isPlayerTurn_
		? *player0Hand_
		: *player1Hand_;

	if (!rule_->IsCheckmate(
		*board_,
		currentHand,
		isPlayerTurn_))
	{
		return;
	}

	isGameOver_ = true;

	// 詰んだ側の相手が勝者
	isPlayerWin_ = !isPlayerTurn_;

	gameOverReason_ =
		GameOverReason::CHECKMATE;

	gameOverFrame_ = 180;

}

void MiniShogi::UpdateGameOver(void)
{
	if (gameOverFrame_ > 0)
	{
		gameOverFrame_--;
		return;
	}

	isReturn_ = true;
}

void MiniShogi::CpuUpdate(void)
{
	CpuMove move =
		cpu_->Think(
			*board_,
			*player1Hand_,
			*rule_);

	if (move.pieceType == PieceType::NONE)
	{
		// 詰みまたは合法手なしを共通処理で判定
		CheckGameOver();
		return;
	}

	if (move.isDrop)
	{
		ExecuteDrop(
			move.pieceType,
			move.toX,
			move.toY);
	}
	else
	{
		ExecuteCpuMove(
			move.fromX,
			move.fromY,
			move.toX,
			move.toY,
			move.isPromote);
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

		capturePiece.isPlayer_ = isPlayerTurn_;
		capturePiece.isPromote_ = false;

		player1Hand_->AddPiece(capturePiece.type_);
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
