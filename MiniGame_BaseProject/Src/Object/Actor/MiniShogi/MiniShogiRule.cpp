#include "MiniShogiRule.h"

MiniShogiRule::MiniShogiRule(void)
{
}

MiniShogiRule::~MiniShogiRule(void)
{
}

bool MiniShogiRule::CanSelectPiece(const MiniShogiBoard& board, int x, int y, bool isPlayerTurn)
{
    if (!board.IsExistPiece(x, y))
    {
        return false;
    }

    const Piece& piece = board.GetPiece(x, y);
    return piece.isPlayer_ == isPlayerTurn;
}

std::vector<MoveData> MiniShogiRule::GetMoveList(const MiniShogiBoard& board, int x, int y)
{
    std::vector<MoveData> moveList;

    if (!board.IsExistPiece(x, y)) return moveList;

    const Piece& piece = board.GetPiece(x, y);

    switch (piece.type_)
    {
    case PieceType::OU:
    {
        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            for (int offsetX = -1; offsetX <= 1; offsetX++)
            {
                if (offsetX == 0 && offsetY == 0) continue;

                AddMove(moveList,
                    board,
                    x + offsetX,
                    y + offsetY,
                    piece.isPlayer_
                );
            }
        }
    }
        break;

    case PieceType::HISHA:
    {
        AddLineMove(moveList, board, x, y, 0, -1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, 0, 1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, -1, 0, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, 1, 0, piece.isPlayer_);
    }
        break;
    case PieceType::KAKUGYO:
    {
        AddLineMove(moveList, board, x, y, -1, -1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, 1, -1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, -1, 1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, 1, 1, piece.isPlayer_);
    }
        break;

    case PieceType::KIN:
    {
        int direction = piece.isPlayer_ ? -1 : 1;

        AddMove(moveList, board, x, y + direction, piece.isPlayer_);

        AddMove(moveList, board, x - 1, y + direction, piece.isPlayer_);
        AddMove(moveList, board, x + 1, y + direction, piece.isPlayer_);

        AddMove(moveList, board, x - 1, y, piece.isPlayer_);
        AddMove(moveList, board, x + 1, y, piece.isPlayer_);

        AddMove(moveList, board, x, y - direction, piece.isPlayer_);
    }
        break;
    case PieceType::GIN:
    {
        int direction = piece.isPlayer_ ? -1 : 1;

        AddMove(moveList, board, x, y + direction, piece.isPlayer_);

        AddMove(moveList, board, x - 1, y + direction, piece.isPlayer_);
        AddMove(moveList, board, x + 1, y + direction, piece.isPlayer_);

        AddMove(moveList, board, x - 1, y - direction, piece.isPlayer_);
        AddMove(moveList, board, x + 1, y - direction, piece.isPlayer_);
    }
        break;
    case PieceType::FU:
    {
        int direction = piece.isPlayer_ ? -1 : 1;

        AddMove(moveList, board, x, y + direction, piece.isPlayer_);
    }
        break;
    default:
        break;
    }

    return moveList;
}

void MiniShogiRule::AddMove(std::vector<MoveData>& moveList, const MiniShogiBoard& board, int x, int y, bool isPlayer)
{
    if (!IsInsideBoard(x, y)) return;

    if (board.IsExistPiece(x, y))
    {
        const Piece& piece = board.GetPiece(x, y);

        if (piece.isPlayer_ == isPlayer) return;
    }

    moveList.push_back({ x,y });
}

void MiniShogiRule::AddLineMove(std::vector<MoveData>& moveList, const MiniShogiBoard& board, int x, int y, int offsetX, int offsetY, bool isPlayer)
{
    int moveX = x + offsetX;
    int moveY = y + offsetY;

    while (IsInsideBoard(moveX, moveY))
    {
        const Piece& piece = board.GetPiece(moveX, moveY);

        if (piece.isPlayer_ != isPlayer) moveList.push_back({ moveX,moveY });

        break;
    }

    moveList.push_back({ moveX,moveY });

    moveX += offsetX;
    moveY += offsetY;
}

bool MiniShogiRule::IsInsideBoard(int x, int y)
{
    return x >= 0 && x < 5
        && y >= 0 && y < 5;
}
