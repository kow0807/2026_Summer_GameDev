#pragma once
#include <vector>

#include "PieceType.h"

struct HandPiece
{
	PieceType type_;
};

class Hand
{
public:

	Hand(void);
	~Hand(void);

	void AddPiece(PieceType type);

	void RemovePiece(PieceType type);

	bool HasPiece(PieceType type) const;
	bool HasPiece(int index) const;

	bool IsEmpty(void) const;

	int GetPieceCount(void) const;

	const HandPiece& GetPiece(int index) const;

	const std::vector<HandPiece>& GetPieceList(void) const;

private:

	int FindIndex(PieceType type) const;

	std::vector<HandPiece> handPieceList_;
};

