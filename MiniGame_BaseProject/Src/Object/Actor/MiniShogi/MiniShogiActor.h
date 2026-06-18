#pragma once

#include <array>
#include <memory>

#include "PieceType.h"
#include "MiniShogiBoard.h"


class MiniShogiGrid;
class MiniShogiPiece;
class Cursor;
class Selector;
class ResourceManager;

class MiniShogiActor
{
public:

	static constexpr int PIECE_COUNT = 12;

	MiniShogiActor(MiniShogiBoard* board_, Cursor* cursor, Selector* selector, const bool& isPlayerTurn);
	~MiniShogiActor(void);

	void Init(void);
	void Update(void);
	void Draw(void);

private:

	MiniShogiBoard* board_;
	Cursor* cursor_;
	Selector* selector_;
	const bool& isPlayerTurn_;
	std::unique_ptr<MiniShogiGrid> grid_;
	std::array<std::unique_ptr<MiniShogiPiece>, PIECE_COUNT> pieces_;

	ResourceManager& resMng_;
	
	int ouH_;
	int gyokuH_;
	int kinH_;
	int ginH_;
	int kakuH_;
	int hishaH_;
	int fuH_;

	VECTOR boardOffset_;
	float cellSize_;

	void SyncPieceActor(void);
	int GetPrototypeModelHandle(PieceType type) const;

	void DrawCursor(void);
	void DrawSelectMarker(void);
	void DrawMoveList(void);
	VECTOR GetCellCenter(int x, int y) const;

	unsigned int GetCursorColor(void) const;
};

