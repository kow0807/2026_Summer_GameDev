#include "MiniShogiRule.h"
#include <cstdlib>
#include <cmath>

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

bool MiniShogiRule::CanDropPiece(const MiniShogiBoard& board, int x, int y) const
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
    bool isPlayerTurn) const
{
    std::vector<MoveData> dropList;
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (!CanDropPiece(board, x, y))
                continue;

            if (pieceType == PieceType::FU)
            {
                if (IsNifu(board, x, isPlayerTurn))
                    continue;

                if (isPlayerTurn && y == 0)
                    continue;

                if (!isPlayerTurn && y == 4)
                    continue;
            }

            dropList.push_back({ x, y });
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
        // 王が存在しない異常盤面
        return true;
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

            // 自分側の駒は対象外
            if (piece.isPlayer_ == playerSide)
            {
                continue;
            }

            if (CanAttackSquare(
                board,
                x,
                y,
                kingX,
                kingY))
            {
                return true;
            }
        }
    }

    return false;
}

bool MiniShogiRule::IsLegalMove(const MiniShogiBoard& board, int fromX, int fromY, int toX, int toY) const
{
    if (!IsInsideBoard(fromX, fromY) ||
        !IsInsideBoard(toX, toY))
    {
        return false;
    }

    if (!board.IsExistPiece(fromX, fromY))
    {
        return false;
    }

    const Piece& movePiece =
        board.GetPiece(fromX, fromY);

    if (board.IsExistPiece(toX, toY))
    {
        const Piece& targetPiece =
            board.GetPiece(toX, toY);

        if (targetPiece.type_ == PieceType::OU)
        {
            return false;
        }
    }

    //----------------------------------
    // 駒本来の移動先に含まれるか
    //----------------------------------
    const std::vector<MoveData> moveList =
        GetMoveList(
            board,
            fromX,
            fromY);

    bool found = false;

    for (const MoveData& move : moveList)
    {
        if (move.x_ == toX &&
            move.y_ == toY)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        return false;
    }

    //----------------------------------
    // 移動後の盤面を作る
    //----------------------------------
    MiniShogiBoard copy = board;

    copy.SetPiece(
        toX,
        toY,
        movePiece);

    copy.RemovePiece(
        fromX,
        fromY);

    // 自玉が王手なら違法手
    return !IsCheck(
        copy,
        movePiece.isPlayer_);
}

bool MiniShogiRule::IsNifu(const MiniShogiBoard& board, int x, bool isPlayerTurn) const
{
    for (int y = 0; y < 5; y++)
    {
        if (!board.IsExistPiece(x, y))
        {
            continue;
        }

        const Piece& piece = board.GetPiece(x, y);

        if (piece.isPlayer_ == isPlayerTurn &&
            piece.type_ == PieceType::FU &&
            !piece.isPromote_)
        {
            return true;
        }
    }

    return false;
}

bool MiniShogiRule::CanDrop(const MiniShogiBoard& board, PieceType pieceType, int x, int y, bool isPlayerTurn, DropError& error) const
{
    if (!IsInsideBoard(x, y))
    {
        error = DropError::OUTSIDE_BOARD;
        return false;
    }

    if (board.IsExistPiece(x, y))
    {
        error = DropError::EXIST_PIECE;
        return false;
    }

    if (pieceType == PieceType::FU)
    {
        if (IsNifu(board, x, isPlayerTurn))
        {
            error = DropError::NIFU;
            return false;
        }

        if (isPlayerTurn && y == 0)
        {
            error = DropError::DEAD_PIECE;
            return false;
        }

        if (!isPlayerTurn && y == 4)
        {
            error = DropError::DEAD_PIECE;
            return false;
        }
    }

    error = DropError::NONE;
    return true;
}


const char* MiniShogiRule::GetDropErrorMessage(DropError error) const
{
    switch (error)
    {
    case DropError::OUTSIDE_BOARD:
        return "盤外のため配置できません";

    case DropError::EXIST_PIECE:
        return "駒があるため配置できません";

    case DropError::NIFU:
        return "二歩のため配置できません";

    case DropError::DEAD_PIECE:
        return "行き所のない駒になるため配置できません";

    default:
        return "";
    }
}

bool MiniShogiRule::HasAnyLegalMove(const MiniShogiBoard& board, const Hand& hand, bool playerSide) const
{
    //----------------------------------
    // 盤上の合法手
    //----------------------------------
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

            if (piece.isPlayer_ != playerSide)
            {
                continue;
            }

            const std::vector<MoveData> legalList =
                GetLegalMoveList(board, x, y);

            if (!legalList.empty())
            {
                return true;
            }
        }
    }

    //----------------------------------
    // 持ち駒の合法手
    //----------------------------------
    for (int i = 0;
        i < hand.GetPieceCount();
        i++)
    {
        const PieceType pieceType =
            hand.GetPiece(i).type_;

        const std::vector<MoveData> dropList =
            GetDropList(
                board,
                pieceType,
                playerSide);

        for (const MoveData& move : dropList)
        {
            MiniShogiBoard testBoard = board;

            const Piece dropPiece
            {
                pieceType,
                playerSide,
                false
            };

            testBoard.SetPiece(
                move.x_,
                move.y_,
                dropPiece);

            // 駒を打つことで王手を解除できる
            if (!IsCheck(testBoard, playerSide))
            {
                return true;
            }
        }
    }

    return false;
}

bool MiniShogiRule::IsCheckmate(const MiniShogiBoard& board, const Hand& hand, bool playerSide) const
{
    // 王手されていなければ詰みではない
    if (!IsCheck(board, playerSide))
    {
        return false;
    }

    // 王手されていて合法手がなければ詰み
    return !HasAnyLegalMove(
        board,
        hand,
        playerSide);
}

bool MiniShogiRule::IsKingExist(const MiniShogiBoard& board, bool playerSide) const
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
                return true;
            }
        }
    }

    return false;
}

std::vector<MoveData> MiniShogiRule::GetLegalMoveList(const MiniShogiBoard& board, int x, int y) const
{
    std::vector<MoveData> legalMoveList;

    if (!board.IsExistPiece(x, y))
    {
        return legalMoveList;
    }

    const std::vector<MoveData> moveList =
        GetMoveList(board, x, y);

    for (const MoveData& move : moveList)
    {
        if (IsLegalMove(
            board,
            x,
            y,
            move.x_,
            move.y_))
        {
            legalMoveList.push_back(move);
        }
    }

    return legalMoveList;
}

std::vector<MoveData> MiniShogiRule::GetLegalDropList(const MiniShogiBoard& board, PieceType pieceType, bool playerSide) const
{
    std::vector<MoveData> legalDropList;

    const std::vector<MoveData> dropList =
        GetDropList(
            board,
            pieceType,
            playerSide);

    for (const MoveData& move : dropList)
    {
        MiniShogiBoard testBoard = board;

        const Piece dropPiece
        {
            pieceType,
            playerSide,
            false
        };

        testBoard.SetPiece(
            move.x_,
            move.y_,
            dropPiece);

        // 打ったあとに自玉が王手なら違法
        if (IsCheck(testBoard, playerSide))
        {
            continue;
        }

        legalDropList.push_back(move);
    }

    return legalDropList;
}

void MiniShogiRule::AddMove(std::vector<MoveData>& moveList, const MiniShogiBoard& board, int x, int y, bool isPlayer) const
{
    if (!IsInsideBoard(x, y))
    {
        return;
    }

    if (board.IsExistPiece(x, y))
    {
        const Piece& piece =
            board.GetPiece(x, y);

        if (piece.isPlayer_ == isPlayer)
        {
            return;
        }
    }

    moveList.push_back({ x, y });
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

bool MiniShogiRule::CanAttackSquare(const MiniShogiBoard& board, int fromX, int fromY, int targetX, int targetY) const
{
    if (!IsInsideBoard(fromX, fromY) ||
        !IsInsideBoard(targetX, targetY))
    {
        return false;
    }

    if (!board.IsExistPiece(fromX, fromY))
    {
        return false;
    }

    const Piece& piece =
        board.GetPiece(fromX, fromY);

    const int dx = targetX - fromX;
    const int dy = targetY - fromY;

    const int direction =
        piece.isPlayer_ ? -1 : 1;

    switch (piece.type_)
    {
    case PieceType::OU:
        return abs(dx) <= 1 &&
            abs(dy) <= 1 &&
            !(dx == 0 && dy == 0);

    case PieceType::KIN:
        return
            (dx == 0 && dy == direction) ||
            (abs(dx) == 1 && dy == direction) ||
            (abs(dx) == 1 && dy == 0) ||
            (dx == 0 && dy == -direction);

    case PieceType::GIN:
        if (piece.isPromote_)
        {
            return
                (dx == 0 && dy == direction) ||
                (abs(dx) == 1 && dy == direction) ||
                (abs(dx) == 1 && dy == 0) ||
                (dx == 0 && dy == -direction);
        }

        return
            (dx == 0 && dy == direction) ||
            (abs(dx) == 1 && dy == direction) ||
            (abs(dx) == 1 && dy == -direction);

    case PieceType::FU:
        if (piece.isPromote_)
        {
            return
                (dx == 0 && dy == direction) ||
                (abs(dx) == 1 && dy == direction) ||
                (abs(dx) == 1 && dy == 0) ||
                (dx == 0 && dy == -direction);
        }

        return dx == 0 &&
            dy == direction;

    case PieceType::HISHA:
    {
        if (piece.isPromote_ &&
            abs(dx) == 1 &&
            abs(dy) == 1)
        {
            return true;
        }

        if (dx == 0 && dy != 0)
        {
            const int stepY = dy > 0 ? 1 : -1;

            return IsLineClear(
                board,
                fromX,
                fromY,
                targetX,
                targetY,
                0,
                stepY);
        }

        if (dy == 0 && dx != 0)
        {
            const int stepX = dx > 0 ? 1 : -1;

            return IsLineClear(
                board,
                fromX,
                fromY,
                targetX,
                targetY,
                stepX,
                0);
        }

        return false;
    }

    case PieceType::KAKUGYO:
    {
        if (piece.isPromote_ &&
            ((abs(dx) == 1 && dy == 0) ||
                (dx == 0 && abs(dy) == 1)))
        {
            return true;
        }

        if (abs(dx) == abs(dy) &&
            dx != 0)
        {
            const int stepX =
                dx > 0 ? 1 : -1;

            const int stepY =
                dy > 0 ? 1 : -1;

            return IsLineClear(
                board,
                fromX,
                fromY,
                targetX,
                targetY,
                stepX,
                stepY);
        }

        return false;
    }

    default:
        return false;
    }
}

bool MiniShogiRule::IsLineClear(const MiniShogiBoard& board, int fromX, int fromY, int targetX, int targetY, int stepX, int stepY) const
{
    int x = fromX + stepX;
    int y = fromY + stepY;

    while (x != targetX || y != targetY)
    {
        if (!IsInsideBoard(x, y))
        {
            return false;
        }

        if (board.IsExistPiece(x, y))
        {
            return false;
        }

        x += stepX;
        y += stepY;
    }

    return true;
}

