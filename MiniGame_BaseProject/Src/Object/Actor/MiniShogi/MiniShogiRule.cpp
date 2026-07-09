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

bool MiniShogiRule::CanSelectHandPiece(const Hand& hand, int handIndex)
{
    if (hand.IsEmpty())
    {
        return false;
    }

    return handIndex >= 0 &&
        handIndex < hand.GetPieceCount();
}

bool MiniShogiRule::CanDropPiece(const MiniShogiBoard& board, int x, int y)
{
    return IsInsideBoard(x, y) &&
        !board.IsExistPiece(x, y);
}

std::vector<MoveData> MiniShogiRule::GetMoveList(const MiniShogiBoard& board, int x, int y) const
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

        if (piece.isPromote_)
        {
            AddMove(moveList, board, x - 1, y - 1, piece.isPlayer_);
            AddMove(moveList, board, x + 1, y - 1, piece.isPlayer_);
            AddMove(moveList, board, x - 1, y + 1, piece.isPlayer_);
            AddMove(moveList, board, x + 1, y + 1, piece.isPlayer_);
        }
    }
        break;
    case PieceType::KAKUGYO:
    {
        AddLineMove(moveList, board, x, y, -1, -1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, 1, -1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, -1, 1, piece.isPlayer_);
        AddLineMove(moveList, board, x, y, 1, 1, piece.isPlayer_);

        if (piece.isPromote_)
        {
            AddMove(moveList, board, x, y - 1, piece.isPlayer_);
            AddMove(moveList, board, x, y + 1, piece.isPlayer_);
            AddMove(moveList, board, x - 1, y, piece.isPlayer_);
            AddMove(moveList, board, x + 1, y, piece.isPlayer_);
        }
    }
        break;

    case PieceType::KIN:
    {
        AddKinMove(
            moveList,
            board,
            x,
            y,
            piece.isPlayer_
        );
    }
        break;
    case PieceType::GIN:
    {
        if (piece.isPromote_)
        {
            AddKinMove(
                moveList,
                board,
                x,
                y,
                piece.isPlayer_
            );
        }
        else
        {
            int direction = piece.isPlayer_ ? -1 : 1;

            AddMove(moveList, board, x, y + direction, piece.isPlayer_);

            AddMove(moveList, board, x - 1, y + direction, piece.isPlayer_);
            AddMove(moveList, board, x + 1, y + direction, piece.isPlayer_);

            AddMove(moveList, board, x - 1, y - direction, piece.isPlayer_);
            AddMove(moveList, board, x + 1, y - direction, piece.isPlayer_);
        }
    }
        break;
    case PieceType::FU:
    {
        if (piece.isPromote_)
        {
            AddKinMove(
                moveList,
                board,
                x,
                y,
                piece.isPlayer_);
        }
        else
        {
            int direction = piece.isPlayer_ ? -1 : 1;

            AddMove(
                moveList,
                board,
                x,
                y + direction,
                piece.isPlayer_);
        }
    }
        break;
    default:
        break;
    }

    return moveList;
}

std::vector<MoveData> MiniShogiRule::GetDropList(
    const MiniShogiBoard& board,
    PieceType pieceType,
    bool isPlayerTurn)
{
    std::vector<MoveData> dropList;
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (CanDropPiece(board, x, y))
            {
                dropList.push_back({ x,y });
            }
        }
    }

	return dropList;
}

bool MiniShogiRule::CanPromote(const Piece& piece, int fromY, int toY) const
{
    if (piece.isPromote_)
    {
        return false;
    }

    switch (piece.type_)
    {
    case PieceType::GIN:
    case PieceType::KAKUGYO:
	case PieceType::HISHA:
    case PieceType::FU:
        break;

    default:
        return false;
    }

    return IsPromoteZone(piece.isPlayer_, fromY) ||
           IsPromoteZone(piece.isPlayer_, toY);
}

bool MiniShogiRule::MustPromote(const Piece& piece, int toY) const
{
    if (piece.isPromote_)
    {
        return false;
    }

	// 歩は最終段に行ったら強制成り
    switch (piece.type_)
    {
    case PieceType::FU:

        if (piece.isPlayer_)
        {
            return toY == 0;
        }
        else
        {
			return toY == 4;
        }

    default:
        break;
    }

	return false;
}

bool MiniShogiRule::IsPromoteZone(bool isPlayer, int y) const
{
    if (isPlayer)
    {
        return y == 0;
    }

	return y == 4;
}

void MiniShogiRule::Promote(Piece& piece) const
{
	piece.isPromote_ = true;
}

bool MiniShogiRule::IsCheck(const MiniShogiBoard& board, bool playerSide) const
{
    int kingX = -1;
    int kingY = -1;

    if (!FindKing(
        board,
        playerSide,
        kingX,
        kingY))
    {
        return false;
    }

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (!board.IsExistPiece(x, y))
            {
                continue;
            }

            const Piece& piece =
                board.GetPiece(x, y);

            if (piece.isPlayer_ == playerSide)
            {
                continue;
            }

            std::vector<MoveData> moveList =
                GetMoveList(board, x, y);

            for (const MoveData& move : moveList)
            {
                if (move.x_ == kingX &&
                    move.y_ == kingY)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool MiniShogiRule::IsCheck(const MiniShogiBoard& board, bool isPlayerTurn)
{
    int kingX = -1;
    int kingY = -1;

    // 自分の王を探す
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (!board.IsExistPiece(x, y))
            {
                continue;
            }

            const Piece& piece =
                board.GetPiece(x, y);

            if (piece.type_ == PieceType::OU &&
                piece.isPlayer_ == isPlayerTurn)
            {
                kingX = x;
                kingY = y;
                break;
            }
        }
    }

    // 王が存在しない
    if (kingX == -1)
    {
        return false;
    }

    // 相手の全駒を調べる
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (!board.IsExistPiece(x, y))
            {
                continue;
            }

            const Piece& piece =
                board.GetPiece(x, y);

            // 自分の駒なら無視
            if (piece.isPlayer_ == isPlayerTurn)
            {
                continue;
            }

            std::vector<MoveData> moveList =
                GetMoveList(board, x, y);

            for (const MoveData& move : moveList)
            {
                if (move.x_ == kingX &&
                    move.y_ == kingY)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool MiniShogiRule::IsLegalMove(const MiniShogiBoard& board, int fromX, int fromY, int toX, int toY) const
{
    // 盤面をコピー
    MiniShogiBoard copy = board;

    Piece movePiece = copy.GetPiece(fromX, fromY);

    copy.SetPiece(toX, toY, movePiece);
    copy.RemovePiece(fromX, fromY);

    // 自分の王が王手なら違法手
    return !IsCheck(copy, movePiece.isPlayer_);
}

void MiniShogiRule::AddMove(std::vector<MoveData>& moveList, const MiniShogiBoard& board, int x, int y, bool isPlayer) const
{
    if (!IsInsideBoard(x, y)) return;

    if (board.IsExistPiece(x, y))
    {
        const Piece& piece = board.GetPiece(x, y);

        if (piece.isPlayer_ == isPlayer) return;
    }

    moveList.push_back({ x,y });
}

void MiniShogiRule::AddLineMove(std::vector<MoveData>& moveList, const MiniShogiBoard& board, int x, int y, int offsetX, int offsetY, bool isPlayer) const
{
    int moveX = x + offsetX;
    int moveY = y + offsetY;

    while (IsInsideBoard(moveX, moveY))
    {
		// 盤外チェック
        if(!board.IsExistPiece(moveX, moveY))
        {
            moveList.push_back({ moveX,moveY });

			moveX += offsetX;
            moveY+= offsetY;
			continue;
        }

        const Piece& piece = board.GetPiece(moveX, moveY);

        // 敵駒ならとれる
        if (piece.isPlayer_ != isPlayer)
        {
            moveList.push_back({ moveX,moveY });
        }

        break;
    }
}

bool MiniShogiRule::IsInsideBoard(int x, int y) const
{
    return x >= 0 && x < 5
        && y >= 0 && y < 5;
}

void MiniShogiRule::AddKinMove(std::vector<MoveData>& moveList, const MiniShogiBoard& board, int x, int y, bool isPlayer) const
{
	int direction = isPlayer ? -1 : 1;

	AddMove(moveList, board, x, y + direction, isPlayer);

    AddMove(moveList, board, x - 1, y + direction, isPlayer);
    AddMove(moveList, board, x + 1, y + direction, isPlayer);

    AddMove(moveList, board, x - 1, y, isPlayer);
    AddMove(moveList, board, x + 1, y, isPlayer);

    AddMove(moveList, board, x, y - direction, isPlayer);
}

bool MiniShogiRule::FindKing(const MiniShogiBoard& board, bool playerSide, int& kingX, int& kingY) const
{
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (!board.IsExistPiece(x, y))
            {
                continue;
            }

            const Piece& piece =
                board.GetPiece(x, y);

            if (piece.type_ == PieceType::OU &&
                piece.isPlayer_ == playerSide)
            {
                kingX = x;
                kingY = y;
                return true;
            }
        }
    }

    return false;
}
