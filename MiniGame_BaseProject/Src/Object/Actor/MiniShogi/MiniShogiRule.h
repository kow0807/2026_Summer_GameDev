#pragma once

#include <vector>

#include "MiniShogiBoard.h"
#include "Hand.h"
#include "MoveData.h"

class MiniShogiRule
{
public:

	MiniShogiRule(void);
	~MiniShogiRule(void);

	bool CanSelectPiece(const MiniShogiBoard& board, int x, int y, bool isPlayerTurn);

	bool CanSelectHandPiece(const Hand& hand, int handIndex);

	bool CanDropPiece(const MiniShogiBoard& board, int x, int y);

	std::vector<MoveData> GetMoveList(
		const MiniShogiBoard& board,
		int x,
		int y
	) const;

	std::vector<MoveData> GetDropList(
		const MiniShogiBoard& board,
		PieceType pieceType,
		bool isPlayerTurn
	);

	// ãÓÇê¨ÇÍÇÈÇ©Ç«Ç§Ç©ÇîªíËÇ∑ÇÈ
	bool CanPromote(const Piece& piece, int fromY, int toY) const;

	// ãÓÇê¨ÇÁÇ»ÇØÇÍÇŒÇ»ÇÁÇ»Ç¢Ç©Ç«Ç§Ç©ÇîªíËÇ∑ÇÈ
	bool MustPromote(const Piece& piece, int toY) const;

	// ê¨ÇÍÇÈÇ©Ç«Ç§Ç©ÇîªíËÇ∑ÇÈ
	bool IsPromoteZone(bool isPlayer, int y) const;

	// ãÓÇê¨ÇÁÇπÇÈ
	void Promote(Piece& piece) const;

	bool IsCheck(const MiniShogiBoard& board, bool playerSide) const;

	bool IsCheck(const MiniShogiBoard& board, bool isPlayerTurn);

	bool IsLegalMove(
		const MiniShogiBoard& board,
		int fromX,
		int fromY,
		int toX,
		int toY
	) const;

private:

	void AddMove(std::vector<MoveData>& moveList,
		const MiniShogiBoard& board,
		int x,
		int y,
		bool isPlayer
	) const;

	void AddLineMove(
		std::vector<MoveData>& moveList,
		const MiniShogiBoard& board,
		int x,
		int y,
		int offsetX,
		int offsetY,
		bool isPlayer
	) const;

	bool IsInsideBoard(int x, int y) const;

	void AddKinMove(
		std::vector<MoveData>& moveList,
		const MiniShogiBoard& board,
		int x,
		int y,
		bool isPlayer) const;

	bool FindKing(const MiniShogiBoard& board,
		bool playerSide,
		int& kingX,
		int& kingY) const;
};

