#pragma once

#include <array>
#include <memory>

#include "PieceType.h"
#include "MiniShogiBoard.h"


class MiniShogiGrid;
class MiniShogiPiece;
class Cursor;
class Selector;
class Hand;
class HandActor;
class ResourceManager;

class MiniShogiActor
{
public:

	static constexpr int PIECE_COUNT = 12;

	MiniShogiActor(MiniShogiBoard* board_, Hand* player0Hand, Hand* player1Hand, Cursor* cursor, Selector* selector, const bool& isPlayerTurn);
	~MiniShogiActor(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	const HandActor* GetPlayerHandActor(void) const;
	const HandActor* GetEnemyHandActor(void) const;

private:

	MiniShogiBoard* board_;
	Cursor* cursor_;
	Selector* selector_;
	const bool& isPlayerTurn_;
	std::unique_ptr<MiniShogiGrid> grid_;
	std::array<std::unique_ptr<MiniShogiPiece>, PIECE_COUNT> pieces_;

	Hand* player0Hand_;
	Hand* enemyHand_;

	std::unique_ptr<HandActor> playerHandActor_;
	std::unique_ptr<HandActor> enemyHandActor_;

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

	void DrawBoardCursor(void);

	void DrawHandCursor(void);

	unsigned int GetHandCursorColor(void) const;
};

