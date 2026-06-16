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
	// 空初期化
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			board[y][x] =
			{
				PieceType::NONE,
				false,
				false
			};
		}
	}

	// 上側（敵）

	board[0][0] = { PieceType::HISHA,   false,false };
	board[0][1] = { PieceType::KAKUGYO, false,false };
	board[0][2] = { PieceType::GIN, false,false };
	board[0][3] = { PieceType::KIN,   false,false };
	board[0][4] = { PieceType::OU,   false,false };

	board[1][2] = { PieceType::FU, false,false };

	// 下側（プレイヤー）

	board[3][2] = { PieceType::FU, true,false };

	board[4][0] = { PieceType::OU,   true,false };
	board[4][1] = { PieceType::KIN,   true,false };
	board[4][2] = { PieceType::GIN, true,false };
	board[4][3] = { PieceType::KAKUGYO, true,false };
	board[4][4] = { PieceType::HISHA,   true,false };
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

void MiniShogiBoard::RemovePiece(int x, int y)
{
	board[y][x] =
	{
		PieceType::NONE,
		false,
		false
	};
}
