#pragma once

#include <vector>

#include "MiniShogiBoard.h"
#include "MoveData.h"

class MiniShogiRule
{
public:

	MiniShogiRule(void);
	~MiniShogiRule(void);

	bool CanSelectPiece(const MiniShogiBoard& board, int x, int y, bool isPlayerTurn);

	std::vector<MoveData> GetMoveList(
		const MiniShogiBoard& board,
		int x,
		int y
	);

private:

	void AddMove(std::vector<MoveData>& moveList,
		const MiniShogiBoard& board,
		int x,
		int y,
		bool isPlayer
	);

	void AddLineMove(
		std::vector<MoveData>& moveList,
		const MiniShogiBoard& board,
		int x,
		int y,
		int offsetX,
		int offsetY,
		bool isPlayer
	);

	bool IsInsideBoard(int x, int y);
};

