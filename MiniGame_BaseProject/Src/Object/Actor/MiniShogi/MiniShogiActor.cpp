#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "Piece.h"
#include "MiniShogiGrid.h"
#include "MiniShogiPiece.h"
#include "Cursor.h"
#include "Selector.h"
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
	Cursor* cursor,
	Selector* selector,
	const bool& isPlayerTurn)
	:
	resMng_(ResourceManager::GetInstance()),
	board_(board),
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

	for (int i = 6; i < 12; i++)
	{
		pieces_[i]->SetRotationY(180);
	}

	SyncPieceActor();
}

void MiniShogiActor::Update(void)
{
	SyncPieceActor();

	if (grid_)
	{
		grid_->Update();
	}

	for (auto& piece : pieces_)
	{

		piece->Update();
	}
}

void MiniShogiActor::Draw(void)
{
	if (grid_)
	{
		grid_->Draw();
	}

	for (auto& piece : pieces_)
	{
		piece->Draw();
	}

	DrawCursor();
	DrawSelectMarker();
	DrawMoveList();
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
				return;
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
	if (cursor_ == nullptr)
	{
		return;
	}

	if (cursor_->GetArea() != CursorArea::BOARD)
	{
		return;
	}
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
	if (!selector_)
	{
		return;
	}

	if (!selector_->IsSelecting())
	{
		return;
	}

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
	if (!selector_->IsSelecting())
	{
		return;
	}

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
