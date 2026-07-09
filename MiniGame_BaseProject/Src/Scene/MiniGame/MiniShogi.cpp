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

#include "MiniShogi.h"

MiniShogi::MiniShogi(void)
	:
	isPlayerTurn_(true)
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
}

void MiniShogi::Update(void)
{

	shogiban_->Update();
	komadai_->Update();

	cursor_->SetHandPieceCount(CursorArea::PLAYER_HAND, actor_->GetPlayerHandActor()->GetPieceCount());
	cursor_->SetHandPieceCount(CursorArea::ENEMY_HAND, actor_->GetEnemyHandActor()->GetPieceCount());

	cursor_->SetPlayerTurn(isPlayerTurn_);

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

	UpdateCameraState();

	board_->Update();

	actor_->Update();


}

void MiniShogi::Draw(void)
{

	//shogiban_->Draw();
	komadai_->Draw();
	actor_->Draw();

}

void MiniShogi::DrawUI(void)
{
	if (promotionState_ != PromotionState::WAIT_SELECT)
	{
		return;
	}

	constexpr int WINDOW_W = 340;
	constexpr int WINDOW_H = 160;

	constexpr int SCREEN_W = 1024;
	constexpr int SCREEN_H = 640;

	const int left = (SCREEN_W - WINDOW_W) / 2;
	const int top = (SCREEN_H - WINDOW_H) / 2;
	const int right = left + WINDOW_W;
	const int bottom = top + WINDOW_H;

	// 背景
	DrawBox(
		left,
		top,
		right,
		bottom,
		GetColor(40, 40, 40),
		TRUE);

	// 枠
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

	//------------------------------------
	// はい
	//------------------------------------

	if (promoteSelect_)
	{
		DrawString(
			left + 70,
			top + 90,
			"はい",
			GetColor(255, 255, 0));
	}
	else
	{
		DrawString(
			left + 70,
			top + 90,
			"はい",
			GetColor(255, 255, 255));
	}

	//------------------------------------
	// いいえ
	//------------------------------------

	if (!promoteSelect_)
	{
		DrawString(
			left + 190,
			top + 90,
			"いいえ",
			GetColor(255, 255, 0));
	}
	else
	{
		DrawString(
			left + 190,
			top + 90,
			"いいえ",
			GetColor(255, 255, 255));
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
}

void MiniShogi::SelectBoardPiece(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	if (!rule_->CanSelectPiece(*board_, x, y, isPlayerTurn_)) return;

	selector_->Select(true);

	selector_->SetSelectPositon(CursorArea::BOARD, x, y, 0);

	selector_->SetSelectPieceType(board_->GetPiece(x, y).type_);

	selector_->SetMoveList(rule_->GetMoveList(*board_, x, y));
}

void MiniShogi::MoveBoardPiece(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	if(!selector_->IsMovePosition(x, y))
	{
		selector_->Select(false);
		return;
	}

	MovePiece(
		selector_->GetSelectX(),
		selector_->GetSelectY(),
		x,
		y
	);

	selector_->Select(false);

	isPlayerTurn_ = !isPlayerTurn_;
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

	if (!rule_->CanDropPiece(*board_, x, y))
	{
		selector_->Select(false);
		return;
	}

	Hand& hand = GetCurrentHand();

	Piece piece
	{
		selector_->GetSelectPieceType(),
		isPlayerTurn_,
		false
	};

	board_->SetPiece(x, y, piece);

	hand.RemovePiece(piece.type_);

	selector_->Select(false);

	isPlayerTurn_ = !isPlayerTurn_;
}

void MiniShogi::MovePiece(int fromX, int fromY, int toX, int toY)
{
	Piece movePiece = board_->GetPiece(fromX, fromY);

	if (rule_->MustPromote(movePiece, toY))
	{
		rule_->Promote(movePiece);
	}
	else if (rule_->CanPromote(movePiece, fromY, toY))
	{
		pendingMovePiece_ = movePiece;

		pendingFromX_ = fromX;
		pendingFromY_ = fromY;

		pendingToX_ = toX;
		pendingToY_ = toY;

		promotionState_ =
			PromotionState::WAIT_SELECT;

		promoteSelect_ = true;

		return;
	}

	// 駒取得
	if (board_->IsExistPiece(toX, toY))
	{
		Piece capturePiece = board_->GetPiece(toX, toY);
		
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

	board_->SetPiece(toX, toY, movePiece);

	board_->RemovePiece(fromX, fromY);

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
