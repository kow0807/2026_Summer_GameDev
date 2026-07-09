#pragma once

enum class CursorArea
{
	BOARD,
	PLAYER_HAND,
	ENEMY_HAND
};

class Cursor
{
public:

	static constexpr int BOARD_WIDTH = 5;
	static constexpr int BOARD_HEIGHT = 5;
	static constexpr int HAND_COLUMN = 2;

	static constexpr int HAND_ROW = 3;
	static constexpr int HAND_COL = 3;
	static constexpr int MAX_HAND_PIECE = HAND_ROW * HAND_COL;

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

	void SetHandPieceCount(CursorArea area, int count);

	void SetBoardPosition(int x, int y);

	void SetPlayerTurn(bool isPlayerTurn);

private:

	CursorArea myArea_;

	int mX_, mY_;

	int playerHandIndex_, enemyHandIndex_;

	int playerHandPieceCount_, enemyHandPieceCount_;

	bool isPlayerTurn_;

	void BoardMoveUp(void);
	void BoardMoveDown(void);
	void BoardMoveLeft(bool isPlayerTurn);
	void BoardMoveRight(bool isPlayerTurn);
	void BoardMoveHorizontal(int dir);

	void HandMoveUp(void);
	void HandMoveDown(void);
	void HandMoveHorizontal(int dir);

	int* GetCurrentHandIndex(void);
	int GetCurrentHandPieceCount(void) const;
};

