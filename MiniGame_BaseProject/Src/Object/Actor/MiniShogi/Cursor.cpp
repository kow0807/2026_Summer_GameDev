#include <DxLib.h>
#include "Cursor.h"

Cursor::Cursor(void)
	:
	myArea_(CursorArea::BOARD),
	mX_(2),
	mY_(4),
	playerHandIndex_(0),
	enemyHandIndex_(0),
	playerHandPieceCount_(0),
	enemyHandPieceCount_(0),
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

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		HandMoveUp();
		break;
	}
}

void Cursor::MoveDown(void)
{	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveDown();
		break;

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		HandMoveDown();
		break;
	}

}

void Cursor::MoveLeft(void)
{
	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveHorizontal(-1);
		break;

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		HandMoveHorizontal(-1);
		break;
	}
}

void Cursor::MoveRight(void)
{
	switch (myArea_)
	{
	case CursorArea::BOARD:
		BoardMoveHorizontal(1);
		break;

	case CursorArea::PLAYER_HAND:
	case CursorArea::ENEMY_HAND:
		HandMoveHorizontal(1);
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
	case CursorArea::PLAYER_HAND:
		return playerHandIndex_;
		break;

	case CursorArea::ENEMY_HAND:
		return enemyHandIndex_;
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
	case CursorArea::PLAYER_HAND:

		playerHandPieceCount_ = count;

		if (playerHandIndex_ >= count)
		{
			playerHandIndex_ =
				count > 0 ? count - 1 : 0;
		}

		break;

	case CursorArea::ENEMY_HAND:

		enemyHandPieceCount_ = count;

		if (enemyHandIndex_ >= count)
		{
			enemyHandIndex_ =
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

void Cursor::SetPlayerTurn(bool isPlayerTurn)
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
	if (mX_ > 0)
	{
		mX_--;
		return;
	}

	// ç∂í[Ç≈é©ï™ÇÃéùÇøãÓÇ÷
	if (isPlayerTurn)
	{
		ChangeArea(CursorArea::PLAYER_HAND);
	}
	else
	{
		ChangeArea(CursorArea::ENEMY_HAND);
	}
}

void Cursor::BoardMoveRight(bool isPlayerTurn)
{
	if (mX_ < BOARD_WIDTH - 1)
	{
		mX_++;
		return;
	}

	// âEí[Ç≈é©ï™ÇÃéùÇøãÓÇ÷
	if (isPlayerTurn)
	{
		ChangeArea(CursorArea::PLAYER_HAND);
	}
	else
	{
		ChangeArea(CursorArea::ENEMY_HAND);
	}
}

void Cursor::BoardMoveHorizontal(int dir)
{
	if (isPlayerTurn_)
	{
		dir = -dir;
	}

	int nextX = mX_ + dir;

	if(nextX >= 0 && nextX < BOARD_WIDTH)
	{
		mX_ = nextX;
		return;
	}

	if (dir < 0)
	{
		if (isPlayerTurn_)
		{
			ChangeArea(CursorArea::PLAYER_HAND);
		}
	}
	else
	{
		if (!isPlayerTurn_)
		{
			ChangeArea(CursorArea::ENEMY_HAND);
		}
	}
}

void Cursor::HandMoveUp(void)
{
	int* index = GetCurrentHandIndex();

	if (index == nullptr)
		return;

	if (*index >= HAND_COL)
	{
		*index -= HAND_COL;
	}
}

void Cursor::HandMoveDown(void)
{
	int* index = GetCurrentHandIndex();

	if (index == nullptr)
		return;

	int next = *index + HAND_COL;

	if (next < GetCurrentHandPieceCount())
	{
		*index = next;
	}
}


void Cursor::HandMoveHorizontal(int dir)
{
	int* index = GetCurrentHandIndex();
	if (index == nullptr)
	{
		return;
	}

	int count = GetCurrentHandPieceCount();

	if (count == 0)
	{
		ChangeArea(CursorArea::BOARD);
		return;
	}

	int row = *index / HAND_COL;
	int col = *index % HAND_COL;

	if (myArea_ == CursorArea::PLAYER_HAND)
	{
		dir = -dir;
	}
	
	int nextCol = col + dir;

	// î’ñ Ç÷ñﬂÇÈ
	if(nextCol < 0 || nextCol >= HAND_COL)
	{
		ChangeArea(CursorArea::BOARD);
		return;
	}

	int nextIndex = row * HAND_COL + nextCol;

	if(nextIndex < count)
	{
		*index = nextIndex;
	}
}

int* Cursor::GetCurrentHandIndex(void)
{
	switch (myArea_)
	{
	case CursorArea::PLAYER_HAND:
		return &playerHandIndex_;

	case CursorArea::ENEMY_HAND:
		return &enemyHandIndex_;

	default:
		return nullptr;
	}
}

int Cursor::GetCurrentHandPieceCount(void) const
{
	switch (myArea_)
	{
	case CursorArea::PLAYER_HAND:
		return playerHandPieceCount_;

	case CursorArea::ENEMY_HAND:
		return enemyHandPieceCount_;

	default:
		return 0;
	}
}
