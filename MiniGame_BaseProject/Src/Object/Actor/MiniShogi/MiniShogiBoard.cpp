#include "MiniShogiBoard.h"

MiniShogiBoard::MiniShogiBoard(void)
{
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			board[y][x] = { PieceType::NONE,false,false };
		}
	}
}

MiniShogiBoard::~MiniShogiBoard(void)
{
}

void MiniShogiBoard::Init(void)
{

}

void MiniShogiBoard::Update(void)
{

}

const Piece& MiniShogiBoard::GetPiece(int x, int y) const
{
	return board[y][x];
}

void MiniShogiBoard::SetPiece(int x, int y, const Piece& piece)
{
	board[y][x] = piece;
}

bool MiniShogiBoard::IsExistPiece(int x, int y) const
{
	return board[y][x].type_ != PieceType::NONE;
}
