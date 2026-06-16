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


	//SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FREE);
	//SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	//SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::QUORIDOR);

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
}

void MiniShogi::Update(void)
{

	shogiban_->Update();
	komadai_->Update();


	InputUpdate();
	SelectUpdate();


	cursor_->Update();
	board_->Update();

}

void MiniShogi::Draw(void)
{

	shogiban_->Draw();
	komadai_->Draw();

}

void MiniShogi::DrawUI(void)
{
}

void MiniShogi::Reset(void)
{
}

void MiniShogi::InputUpdate(void)
{
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgUp(KEY_INPUT_UP)) cursor_->MoveUp();
	if (ins.IsTrgUp(KEY_INPUT_DOWN)) cursor_->MoveDown();
	if (ins.IsTrgUp(KEY_INPUT_LEFT)) cursor_->MoveLeft();
	if (ins.IsTrgUp(KEY_INPUT_RIGHT)) cursor_->MoveRight();
}

void MiniShogi::SelectUpdate(void)
{
	auto& ins = InputManager::GetInstance();

	if (!ins.IsTrgUp(KEY_INPUT_Z)) return;

	int x = cursor_->GetX();
	int y = cursor_->GetY();

	switch (cursor_->GetArea())
	{
	case::CursorArea::BOARD:

		// –¢‘I‘ðŽž
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

			selector_->SetMoveList(
				rule_->GetMoveList(
					*board_,
					x,
					y
				)
			);
		}
		// ‘I‘ð’†
		else
		{
			// ˆÚ“®‰Â”\
			if (selector_->IsMovePosition(x, y))
			{
				Piece movePiece =
					board_->GetPiece(
						selector_->GetSelectX(),
						selector_->GetSelectY()
					);

				board_->SetPiece(
					x,
					y,
					movePiece
				);

				board_->SetPiece(
					selector_->GetSelectX(),
					selector_->GetSelectY(),
					{
						PieceType::NONE,
						false,
						false
					}
				);

				selector_->Select(false);

				isPlayerTurn_ = !isPlayerTurn_;
			}
			else
			{
				selector_->Select(false);
			}
		}
		break;
	case::CursorArea::PLAYER1_HAND:
		break;
	case::CursorArea::PLAYER2_HAND:
		break;
	default:
		break;
	}
}
