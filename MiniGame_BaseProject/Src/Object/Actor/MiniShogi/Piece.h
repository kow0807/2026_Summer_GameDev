#pragma once

#include "PieceType.h"

struct Piece
{
	PieceType type_;

	bool isPlayer_;
	bool isPromote_;
};