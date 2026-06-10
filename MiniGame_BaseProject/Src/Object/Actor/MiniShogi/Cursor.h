#pragma once

enum class CursorArea
{
	BOARD,
	PLAYER1_HAND,
	PLAYER2_HAND
};

class Cursor
{
public:

	static constexpr int BOARD_WIDTH = 5;
	static constexpr int BOARD_HEIGHT = 5;
	static constexpr int HAND_COLUMN = 2;
	static constexpr int HAND_ROW = 3;

	Cursor(void);
	~Cursor(void);

	void Init(void);
	void Update(void);

	void MoveUp(void);
	void MoveDown(void);
	void MoveLeft(void);
	void MoveRight(void);

	void ChangeArea(CursorArea area);
	CursorArea GetArea(void) const;

	int GetX(void) const;
	int GetY(void) const;

	int GetHandIndex(void) const;

private:

	CursorArea myArea_;

	int mX_, mY_;

	int player1HandIndex_, player2HandIndex_;

	void BoardMoveUp(void);
	void BoardMoveDown(void);
	void BoardMoveLeft(void);
	void BoardMoveRight(void);

	void HandMoveUp(void);
	void HandMoveDown(void);
	void HandMoveLeft(void);
	void HandMoveRight(void);
};

