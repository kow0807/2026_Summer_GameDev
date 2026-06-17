#pragma once

#include "Piece.h"

class MiniShogiBoard
{
public:

	MiniShogiBoard(void);
	~MiniShogiBoard(void);

	void Init(void);

	void Update(void);

	const Piece& GetPiece(int x, int y) const;

	void SetPiece(int x, int y, const Piece& piece);

	bool IsExistPiece(int x, int y) const;

	void RemovePiece(int x, int y);

private:

	Piece board[5][5];

};

