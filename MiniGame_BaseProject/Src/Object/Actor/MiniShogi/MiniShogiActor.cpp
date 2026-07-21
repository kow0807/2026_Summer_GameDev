#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "Piece.h"
#include "MiniShogiGrid.h"
#include "MiniShogiPiece.h"
#include "Cursor.h"
#include "Selector.h"
#include "Hand.h"
#include "HandActor.h"
#include "MiniShogiActor.h"

namespace
{
	constexpr int BOARD_SIZE = 5;
	constexpr float BOARD_OFFSET_X = -200.0f;
	constexpr float BOARD_OFFSET_Z = -200.0f;
	constexpr float CELL_SIZE = 100.0f;
}

MiniShogiActor::MiniShogiActor(
	MiniShogiBoard* board,
	Hand* player0Hand, 
	Hand* player1Hand,
	Cursor* cursor,
	Selector* selector,
	const bool& isPlayerTurn)
	:
	resMng_(ResourceManager::GetInstance()),
	board_(board),
	player0Hand_(player0Hand),
	enemyHand_(player1Hand),
	cursor_(cursor),
	selector_(selector),
	isPlayerTurn_(isPlayerTurn),
	ouH_(-1),
	gyokuH_(-1),
	kinH_(-1),
	ginH_(-1),
	kakuH_(-1),
	hishaH_(-1),
	fuH_(-1),
	boardOffset_(VGet(BOARD_OFFSET_X, 0.0f, BOARD_OFFSET_Z)),
	cellSize_(CELL_SIZE)
{

}

MiniShogiActor::~MiniShogiActor(void)
{

}

void MiniShogiActor::Init(void)
{
	grid_ = std::make_unique<MiniShogiGrid>();
	grid_->SetBoardOffset(boardOffset_);
	grid_->SetCellSize(cellSize_);
	grid_->Init();

	ouH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_OU);
	gyokuH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_GYOKU);
	kinH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_KIN);
	ginH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_GIN);
	kakuH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_KAKU);
	hishaH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_HISHA);
	fuH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_FU);

	for (auto& piece : pieces_)
	{
		piece = std::make_unique<MiniShogiPiece>();
		piece->Init();
		piece->SetVisible(false);
	}

	playerHandActor_ = std::make_unique<HandActor>(player0Hand_, true);
	playerHandActor_->SetOrigin(VGet(-420.0f, 0.0f, 100.0f));
	playerHandActor_->Init();

	enemyHandActor_ = std::make_unique<HandActor>(enemyHand_, false);
	enemyHandActor_->SetOrigin(VGet(420.0f, 0.0f, -200.0f));
	enemyHandActor_->Init();

	for (int i = 6; i < 12; i++)
	{
		pieces_[i]->SetRotationY(180);
	}

	SyncPieceActor();
}

void MiniShogiActor::Update(void)
{
	SyncPieceActor();

	if (grid_) grid_->Update();

	for (auto& piece : pieces_)
	{
		piece->Update();
	}

	playerHandActor_->Update();
	enemyHandActor_->Update();
}

void MiniShogiActor::Draw(void)
{
	//if (grid_) grid_->Draw();

	for (auto& piece : pieces_)
	{
		piece->Draw();
	}

	playerHandActor_->Draw();
	enemyHandActor_->Draw();

	DrawBoardCursor();

	DrawHandCursor();
}

const HandActor* MiniShogiActor::GetPlayerHandActor(void) const
{
	return playerHandActor_.get();
}

const HandActor* MiniShogiActor::GetEnemyHandActor(void) const
{
	return enemyHandActor_.get();
}

void MiniShogiActor::SyncPieceActor(void)
{
	int actorIndex = 0;

	for (int y = 0; y < BOARD_SIZE; y++)
	{
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			if (!board_->IsExistPiece(x, y))
			{
				continue;
			}

			if (actorIndex >= PIECE_COUNT)
			{
				break;
			}

			const Piece& piece =
				board_->GetPiece(x, y);

			auto& actor =
				pieces_[actorIndex];

			actor->SetVisible(true);

			actor->SetPiece(piece);

			actor->SetBoardCell(
				x,
				y
			);

			actor->SetBoardOffset(
				boardOffset_
			);

			actor->SetCellSize(
				cellSize_
			);

			actor->SetModelHandle(
				GetPrototypeModelHandle(
					piece.type_
				)
			);

			actorIndex++;
		}
	}

	for (; actorIndex < PIECE_COUNT; actorIndex++)
	{
		pieces_[actorIndex]->SetVisible(false);
	}


}

int MiniShogiActor::GetPrototypeModelHandle(PieceType type) const
{
	switch (type)
	{
	case PieceType::OU:
		return ouH_;
	case PieceType::KIN:
		return kinH_;
	case PieceType::GIN:
		return ginH_;
	case PieceType::KAKUGYO:
		return kakuH_;
	case PieceType::HISHA:
		return hishaH_;
	case PieceType::FU:
		return fuH_;
	default:
		return -1;
	}
}

void MiniShogiActor::DrawCursor(void)
{
	if (cursor_ == nullptr) return;

	if (cursor_->GetArea() != CursorArea::BOARD) return;
	
	float posX =
		boardOffset_.x +
		cursor_->GetX() * cellSize_;

	float posZ =
		boardOffset_.z +
		cursor_->GetY() * cellSize_;

	const float halfSize =
		cellSize_ * 0.5f;

	AsoUtility::DrawBox3DThick(
		VGet(
			posX - halfSize,
			5.0f,
			posZ - halfSize
		),
		VGet(
			posX + halfSize,
			20.0f,
			posZ + halfSize
		),
		3.0f,
		GetCursorColor()
	);
}

void MiniShogiActor::DrawSelectMarker(void)
{
	if (!selector_) return;
	
	if (!selector_->IsSelecting()) return;

	const int x =
		selector_->GetSelectX();

	const int y =
		selector_->GetSelectY();

	const VECTOR center =
		GetCellCenter(x, y);

	const float half =
		cellSize_ * 0.45f;

	AsoUtility::DrawBox3DThick(
		VGet(
			center.x - half,
			5.0f,
			center.z - half
		),
		VGet(
			center.x + half,
			20.0f,
			center.z + half
		),
		4.0f,
		GetColor(
			255,
			255,
			0
		)
	);
}

void MiniShogiActor::DrawMoveList(void)
{
	if (!selector_->IsSelecting()) return;

	const auto& moveList =
		selector_->GetMoveList();

	for (const auto& move : moveList)
	{
		VECTOR center =
			GetCellCenter(
				move.x_,
				move.y_
			);

		DrawSphere3D(
			VGet(
				center.x,
				20.0f,
				center.z
			),
			10.0f,
			16,
			GetColor(
				0,
				255,
				255
			),
			GetColor(
				0,
				255,
				255
			),
			TRUE
		);
	}
}

VECTOR MiniShogiActor::GetCellCenter(int x, int y) const
{
	return VGet(
		boardOffset_.x + (x * cellSize_),
		0.0f,
		boardOffset_.z + (y * cellSize_)
	);
}

unsigned int MiniShogiActor::GetCursorColor(void) const
{
	const int x = cursor_->GetX();
	const int y = cursor_->GetY();

	if (!board_->IsExistPiece(x, y))
	{
		return GetColor(255, 255, 255);
	}

	const Piece& piece =
		board_->GetPiece(x, y);

	if (piece.isPlayer_ == isPlayerTurn_)
	{
		return GetColor(0, 255, 0);
	}

	return GetColor(255, 0, 0);
}

void MiniShogiActor::DrawBoardCursor(void)
{
	DrawCursor();
	DrawSelectMarker();
	DrawMoveList();
}

void MiniShogiActor::DrawHandCursor(void)
{
	if(cursor_->GetArea() == CursorArea::BOARD) return;

	HandActor* handActor = nullptr;

	switch (cursor_->GetArea())
	{
	case CursorArea::PLAYER_HAND:
		handActor = playerHandActor_.get();
		break;
	case CursorArea::ENEMY_HAND:
		handActor = enemyHandActor_.get();
		break;
	default:
		
		break;
	}

	if (handActor == nullptr) return;

	if (handActor->GetPieceCount() == 0)
	{
		return;
	}

	const int index = cursor_->GetHandIndex();
	if(index < 0 || index >= handActor->GetPieceCount())
	{
		return;
	}

	VECTOR pos = handActor->GetCellWorldPosition(index);

	VECTOR min =
	{
		pos.x - 35.0f,
		pos.y,
		pos.z - 35.0f
	};

	VECTOR max =
	{
		pos.x + 35.0f,
		pos.y + 60.0f,
		pos.z + 35.0f
	};

	AsoUtility::DrawBox3DThick(min, max, 2.0f, GetHandCursorColor());
}

unsigned int MiniShogiActor::GetHandCursorColor(void) const
{
	switch (cursor_->GetArea())
	{
	case CursorArea::ENEMY_HAND:

		if (isPlayerTurn_)
		{
			return GetColor(255, 0, 0);
		}

		return GetColor(0, 255, 0);

	case CursorArea::PLAYER_HAND:

		if (!isPlayerTurn_)
		{
			return GetColor(255, 0, 0);
		}

		return GetColor(0, 255, 0);

	default:
		return GetColor(255, 255, 255);
	}
}
