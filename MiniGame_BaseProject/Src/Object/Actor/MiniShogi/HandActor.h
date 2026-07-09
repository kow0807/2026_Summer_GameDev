#pragma once

#include "HandActor.h"
#include <array>
#include <vector>
#include <memory>

#include <DxLib.h>
#include "Piece.h"

class Hand;
class MiniShogiPiece;
class ResourceManager;

class HandActor
{
public:

	static constexpr int HAND_ROW = 3;
	static constexpr int HAND_COL = 3;
	static constexpr int MAX_HAND_PIECE = HAND_ROW * HAND_COL;


	HandActor(Hand* hand, bool isPlayerSide);
	~HandActor(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	void SetOrigin(const VECTOR& origin);
	void SetCellSize(float size);

	VECTOR GetPieceWorldPosition(int index) const;
	VECTOR GetCellWorldPosition(int index) const;
	int GetPieceCount(void) const;
	PieceType GetPieceType(int index) const;

private:

	Hand* hand_;
	bool isPlayerSide_;
	VECTOR origin_;
	float cellSize_;
	ResourceManager& resMng_;

	std::array<std::unique_ptr<MiniShogiPiece>, MAX_HAND_PIECE> pieces_;

	int ouH_;
	int gyokuH_;
	int kinH_;
	int ginH_;
	int kakuH_;
	int hishaH_;
	int fuH_;

	void SyncPieceActor(void);
	int GetModelHandle(PieceType type) const;
	VECTOR CalcWorldPosition(int index) const;
};