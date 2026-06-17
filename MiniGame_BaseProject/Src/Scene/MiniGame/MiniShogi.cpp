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

	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::MINISHOGI);

	shogiban_ = std::make_unique<Shogiban>();
	shogiban_->Init();

	komadai_ = std::make_unique<Komadai>();
	komadai_->Init();

	cursor_ = std::make_unique<Cursor>();
	cursor_->Init();

	selector_ = std::make_unique<Selector>();
	
	
	rule_ = std::make_unique <MiniShogiRule>();

	board_ = std::make_unique<MiniShogiBoard>();
	board_->Init();

	player1Hand_ = std::make_unique<Hand>();
	player2Hand_ = std::make_unique<Hand>();

	actor_ = std::make_unique<MiniShogiActor>(board_.get(), cursor_.get(), selector_.get(),isPlayerTurn_);
	actor_->Init();
}

void MiniShogi::Update(void)
{

	shogiban_->Update();
	komadai_->Update();


	InputUpdate();
	SelectUpdate();
	actor_->Update();

	cursor_->Update();
	board_->Update();

}

void MiniShogi::Draw(void)
{

	//shogiban_->Draw();
	komadai_->Draw();
	actor_->Draw();

}

void MiniShogi::DrawUI(void)
{
}

void MiniShogi::Reset(void)
{
	Setting::GetInstance().SetFullScreen(FALSE);
}

void MiniShogi::InputUpdate(void)
{
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgUp(KEY_INPUT_UP)) cursor_->MoveUp();
	if (ins.IsTrgUp(KEY_INPUT_DOWN)) cursor_->MoveDown();
	if (ins.IsTrgUp(KEY_INPUT_LEFT)) cursor_->MoveRight();
	if (ins.IsTrgUp(KEY_INPUT_RIGHT)) cursor_->MoveLeft();
}

void MiniShogi::SelectUpdate(void)
{
	auto& ins = InputManager::GetInstance();

	if (!ins.IsTrgUp(KEY_INPUT_Z)) return;

	switch (cursor_->GetArea())
	{
	case CursorArea::BOARD:
		SelectBoardUpdate();
		break;
	case CursorArea::PLAYER1_HAND:
	case CursorArea::PLAYER2_HAND:
		SelectHandUpdate();
		break;
	default:
		break;
	}


}

void MiniShogi::SelectBoardUpdate(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	if (!selector_->IsSelecting())
	{
		if (!rule_->CanSelectPiece(
			*board_,
			x,
			y,
			isPlayerTurn_
		))
		{
			return;
		}

		selector_->Select(true);

		selector_->SetSelectPositon(
			CursorArea::BOARD,
			x,
			y,
			0
		);
		
		selector_->SetSelectPieceType(
			board_->GetPiece(x, y).type_
		);

		selector_->SetMoveList(
			rule_->GetMoveList(
				*board_,
				x,
				y
			)
		);
	}
	else
	{
		if (selector_->IsMovePosition(x, y))
		{
			MovePiece(
				selector_->GetSelectX(),
				selector_->GetSelectY(),
				x,
				y
			);

			selector_->Select(false);
			isPlayerTurn_ = !isPlayerTurn_;
		}
		else
		{
			selector_->Select(false);
		}
	}
}

void MiniShogi::SelectHandUpdate(void)
{
	int x = cursor_->GetX();
	int y = cursor_->GetY();

	Hand& hand = GetCurrentHand();

	if (!selector_->IsSelecting())
	{
		if (!rule_->CanSelectHandPiece(hand, cursor_->GetHandIndex()))
		{
			return;
		}

		selector_->Select(true);

		selector_->SetSelectPositon(
			cursor_->GetArea(),
			0,
			0,
			cursor_->GetHandIndex()
		);

		selector_->SetSelectPieceType(
			hand.GetPiece(cursor_->GetHandIndex()).type_
		);
	}
	else
	{
		if (rule_->CanDropPiece(*board_, x, y))
		{
			Piece dropPiece
			{
				selector_->GetSelectPieceType(),
				isPlayerTurn_,
				false
			};

			board_->SetPiece(x, y, dropPiece);

			hand.RemovePiece(
				selector_->GetSelectPieceType()
			);

			selector_->Select(false);
			isPlayerTurn_ = !isPlayerTurn_;
		}
		else
		{
			selector_->Select(false);
		}
	}
}

void MiniShogi::MovePiece(int fromX, int fromY, int toX, int toY)
{
	Piece movePiece = board_->GetPiece(fromX, fromY);

	// ‹îŽæ“¾
	if (board_->IsExistPiece(toX, toY))
	{
		Piece capturePiece = board_->GetPiece(toX, toY);
		
		capturePiece.isPlayer_ = isPlayerTurn_;
		capturePiece.isPromote_ = false;

		if (isPlayerTurn_)
		{
			player1Hand_->AddPiece(capturePiece.type_);
		}
		else
		{
			player2Hand_->AddPiece(capturePiece.type_);
		}
	}

	board_->SetPiece(toX, toY, movePiece);

	board_->RemovePiece(fromX, fromY);

}

Hand& MiniShogi::GetCurrentHand(void)
{
	if (isPlayerTurn_)
	{
		return *player1Hand_;
	}

	return *player2Hand_;
}

const Hand& MiniShogi::GetCurrentHand(void) const
{
	if (isPlayerTurn_)
	{
		return *player1Hand_;
	}

	return *player2Hand_;
}
