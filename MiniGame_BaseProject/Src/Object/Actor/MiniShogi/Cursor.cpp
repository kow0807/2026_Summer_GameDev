#include <DxLib.h>
#include "Cursor.h"

Cursor::Cursor(void)
	:
	myArea_(CursorArea::BOARD),
	mX_(2),
	mY_(4),
	player1HandIndex_(0),
	player2HandIndex_(0),
	player1HandPieceCount_(0),
	player2HandPieceCount_(0),
	isPlayerTurn_(false)
{
}

Cursor::~Cursor(void)
{
}

void Cursor::Init(void)
{

}

void Cursor::Update(void)
{

}

void Cursor::MoveUp(void)
{
	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveUp();
		break;

	case CursorArea::PLAYER1_HAND:
	case CursorArea::PLAYER2_HAND:
		HandMoveUp();
		break;
	}
}

void Cursor::MoveDown(void)
{
	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveDown();
		break;

	case CursorArea::PLAYER1_HAND:
	case CursorArea::PLAYER2_HAND:
		HandMoveDown();
		break;
	}
}

void Cursor::MoveLeft(void)
{
	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveLeft(isPlayerTurn_);
		break;

	case CursorArea::PLAYER1_HAND:
	case CursorArea::PLAYER2_HAND:
		HandMoveLeft();
		break;
	}
}

void Cursor::MoveRight(void)
{
	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveRight(isPlayerTurn_);
		break;

	case CursorArea::PLAYER1_HAND:
	case CursorArea::PLAYER2_HAND:
		HandMoveRight();
		break;
	}
}

void Cursor::ChangeArea(CursorArea area)
{
	myArea_ = area;
}

CursorArea Cursor::GetArea(void) const
{
	return myArea_;
}

int Cursor::GetX(void) const
{
	return mX_;
}

int Cursor::GetY(void) const
{
	return mY_;
}

int Cursor::GetHandIndex(void) const
{
	switch (myArea_)
	{
	case CursorArea::PLAYER1_HAND:
		return player1HandIndex_;
		break;

	case CursorArea::PLAYER2_HAND:
		return player2HandIndex_;
		break;

	default:
		return 0;
		break;
	}
}

void Cursor::SetHandPieceCount(CursorArea area, int count)
{
	switch (area)
	{
	case CursorArea::PLAYER1_HAND:

		player1HandPieceCount_ = count;

		if (player1HandIndex_ >= count)
		{
			player1HandIndex_ =
				count > 0 ? count - 1 : 0;
		}

		break;

	case CursorArea::PLAYER2_HAND:

		player2HandPieceCount_ = count;

		if (player2HandIndex_ >= count)
		{
			player2HandIndex_ =
				count > 0 ? count - 1 : 0;
		}

		break;
	}
}

void Cursor::SetBoardPosition(int x, int y)
{
	mX_ = x;
	mY_ = y;
}

void Cursor::SetPlayerTurn(bool& isPlayerTurn)
{
	isPlayerTurn_ = isPlayerTurn;
}

void Cursor::BoardMoveUp(void)
{
	if (mY_ > 0)
	{
		mY_--;
	}
}

void Cursor::BoardMoveDown(void)
{
	if (mY_ < BOARD_HEIGHT - 1)
	{
		mY_++;
	}
}

void Cursor::BoardMoveLeft(bool isPlayerTurn)
{
	if (mX_ < BOARD_WIDTH - 1)
	{
		mX_++;
		return;
	}

	if (isPlayerTurn)
	{
		ChangeArea(CursorArea::PLAYER2_HAND);
	}
	else
	{
		ChangeArea(CursorArea::PLAYER1_HAND);
	}
}

void Cursor::BoardMoveRight(bool isPlayerTurn)
{
	if (mX_ > 0)
	{
		mX_--;
		return;
	}

	if (isPlayerTurn)
	{
		ChangeArea(CursorArea::PLAYER1_HAND);
	}
	else
	{
		ChangeArea(CursorArea::PLAYER2_HAND);
	}
}

void Cursor::HandMoveUp(void)
{
	int* index = GetCurrentHandIndex();

	if (*index >= HAND_COL)
	{
		*index -= HAND_COL;
	}
}

void Cursor::HandMoveDown(void)
{
	int* index = GetCurrentHandIndex();

	if (*index + HAND_COL < HAND_COL * HAND_ROW)
	{
		*index += HAND_COL;
	}
}

void Cursor::HandMoveLeft(void)
{
	int* index = GetCurrentHandIndex();

	if (myArea_ == CursorArea::PLAYER1_HAND)
	{
		if (*index > 0)
		{
			(*index)--;
			return;
		}

		ChangeArea(CursorArea::BOARD);
	}
}

void Cursor::HandMoveRight(void)
{
	int* index = GetCurrentHandIndex();

	if (myArea_ == CursorArea::PLAYER1_HAND)
	{
		if (*index + 1 < GetCurrentHandPieceCount())
		{
			(*index)++;
		}

		return;
	}
}

int* Cursor::GetCurrentHandIndex(void)
{
	switch (myArea_)
	{
	case CursorArea::PLAYER1_HAND:
		return &player1HandIndex_;

	case CursorArea::PLAYER2_HAND:
		return &player2HandIndex_;

	default:
		return nullptr;
	}
}

int Cursor::GetCurrentHandPieceCount(void) const
{
	switch (myArea_)
	{
	case CursorArea::PLAYER1_HAND:
		return player1HandPieceCount_;

	case CursorArea::PLAYER2_HAND:
		return player2HandPieceCount_;

	default:
		return 0;
	}
}
