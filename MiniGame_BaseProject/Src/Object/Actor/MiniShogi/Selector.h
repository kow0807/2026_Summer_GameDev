#pragma once

#include <vector>

#include "MoveData.h"
#include "Cursor.h"
#include "PieceType.h"

class Selector
{
public:

	Selector(void);
	~Selector(void);

	void Select(bool flag);

	bool IsSelecting(void) const;

	void SetSelectPositon(
		CursorArea area,
		int x,
		int y,
		int handIndex
	);

	void SetSelectPieceType(PieceType type);

	void SetMoveList(
		const std::vector<MoveData>& moveList
	);

	const std::vector<MoveData>& GetMoveList(void) const;

	bool IsMovePosition(int x, int y) const;

	CursorArea GetSelectArea(void) const;

	int GetSelectX(void) const;
	int GetSelectY(void) const;

	int GetSelectHandIndex(void) const;

	PieceType GetSelectPieceType(void) const;

private:

	bool isSelecting_;

	CursorArea selectArea_;

	int selectX_, selectY_;

	int selectHandIndex_;

	PieceType selectPieceType_;

	std::vector<MoveData> moveList_;
};

