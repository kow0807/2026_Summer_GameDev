#include "Cursor.h"

Cursor::Cursor(void)
	:
	myArea_(CursorArea::BOARD),
	mX_(0),
	mY_(0),
	player1HandIndex_(0),
	player2HandIndex_(0)
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
		BoardMoveLeft();
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
		BoardMoveRight();
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
	return 0;
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

void Cursor::BoardMoveLeft(void)
{
	if (mX_ > 0)
	{
		mX_--;
	}
	else
	{
		ChangeArea(CursorArea::PLAYER2_HAND);
	}
}

void Cursor::BoardMoveRight(void)
{
	if (mX_ < BOARD_WIDTH - 1)
	{
		mX_++;
	}
	else
	{
		ChangeArea(CursorArea(CursorArea::PLAYER1_HAND));
	}
}

void Cursor::HandMoveUp(void)
{
	int* handIndex = nullptr;

	switch (myArea_)
	{
	case CursorArea::BOARD:
		break;
	case CursorArea::PLAYER1_HAND:
		handIndex = &player1HandIndex_;
		break;
	case CursorArea::PLAYER2_HAND:
		handIndex = &player2HandIndex_;
		break;
	default:
		break;
	}

	if (!handIndex)
	{
		return;
	}

	if (*handIndex - 1 >= 0)
	{
		(*handIndex)--;
	}
}

void Cursor::HandMoveDown(void)
{
	int* handIndex = nullptr;

	switch (myArea_)
	{
	case CursorArea::BOARD:
		break;
	case CursorArea::PLAYER1_HAND:
		handIndex = &player1HandIndex_;
		break;
	case CursorArea::PLAYER2_HAND:
		handIndex = &player2HandIndex_;
		break;
	default:
		break;
	}

	if (!handIndex)
	{
		return;
	}

	if (*handIndex + 1 < HAND_COLUMN * HAND_ROW)
	{
		(*handIndex)++;
	}
}

void Cursor::HandMoveLeft(void)
{
	switch (myArea_)
	{
	case CursorArea::PLAYER1_HAND:
		ChangeArea(CursorArea::BOARD);
		break;
	case CursorArea::PLAYER2_HAND:
		break;
	}
}

void Cursor::HandMoveRight(void)
{
	switch (myArea_)
	{
	case CursorArea::PLAYER1_HAND:
		break;
	case CursorArea::PLAYER2_HAND:
		ChangeArea(CursorArea::BOARD);
		break;
	}
}
