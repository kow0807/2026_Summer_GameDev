#include "MiniShogiCpu.h"
#include <algorithm>
#include <cstdlib>

MiniShogiCpu::MiniShogiCpu()
{
}

MiniShogiCpu::~MiniShogiCpu()
{
}

CpuMove MiniShogiCpu::Think(
    const MiniShogiBoard& board,
    const Hand& hand,
    const MiniShogiRule& rule)
{
    std::vector<CpuMove> moveList;

    CreateMoveList(
        board,
        hand,
        rule,
        moveList);

    if (moveList.empty())
    {
        return CpuMove{};
    }

    return FindBestMove(
        moveList,
        board);
}

void MiniShogiCpu::CreateMoveList(
    const MiniShogiBoard& board,
    const Hand& hand,
    const MiniShogiRule& rule,
    std::vector<CpuMove>& moveList)
{
    moveList.clear();

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

            // CPUë§ÇÕfalse
            if (piece.isPlayer_ != CPU_SIDE)
            {
                continue;
            }

            AddBoardMove(
                board,
                rule,
                x,
                y,
                moveList);
        }
    }

    AddDropMove(
        board,
        hand,
        rule,
        moveList);
}

void MiniShogiCpu::AddBoardMove(
    const MiniShogiBoard& board,
    const MiniShogiRule& rule,
    int fromX,
    int fromY,
    std::vector<CpuMove>& moveList)
{
    if (!board.IsExistPiece(fromX, fromY))
    {
        return;
    }

    const Piece& piece =
        board.GetPiece(fromX, fromY);

    if (!IsCpuPiece(piece))
    {
        return;
    }

    const std::vector<MoveData> list =
        rule.GetLegalMoveList(
            board,
            fromX,
            fromY);

    for (const MoveData& move : list)
    {
        //----------------------------------
        // ã≠êßê¨ÇË
        //----------------------------------
        if (rule.MustPromote(
            piece,
            move.y_))
        {
            CpuMove cpuMove{};

            cpuMove.isDrop = false;
            cpuMove.isPromote = true;

            cpuMove.fromX = fromX;
            cpuMove.fromY = fromY;

            cpuMove.toX = move.x_;
            cpuMove.toY = move.y_;

            cpuMove.pieceType =
                piece.type_;

            cpuMove.score = 0;

            moveList.push_back(cpuMove);
            continue;
        }

        //----------------------------------
        // ê¨ÇÁÇ»Ç¢éË
        //----------------------------------
        CpuMove normalMove{};

        normalMove.isDrop = false;
        normalMove.isPromote = false;

        normalMove.fromX = fromX;
        normalMove.fromY = fromY;

        normalMove.toX = move.x_;
        normalMove.toY = move.y_;

        normalMove.pieceType =
            piece.type_;

        normalMove.score = 0;

        moveList.push_back(normalMove);

        //----------------------------------
        // îCà”ê¨ÇË
        //----------------------------------
        if (rule.CanPromote(
            piece,
            fromY,
            move.y_))
        {
            CpuMove promoteMove =
                normalMove;

            promoteMove.isPromote = true;

            moveList.push_back(
                promoteMove);
        }
    }
}

void MiniShogiCpu::AddDropMove(
    const MiniShogiBoard& board,
    const Hand& hand,
    const MiniShogiRule& rule,
    std::vector<CpuMove>& moveList)
{
    for (int i = 0;
        i < hand.GetPieceCount();
        i++)
    {
        PieceType type =
            hand.GetPiece(i).type_;

        auto list =
            rule.GetLegalDropList(
                board,
                type,
                false);

        for (const auto& move : list)
        {
            CpuMove cpuMove;

            cpuMove.isDrop = true;
            cpuMove.isPromote = false;

            cpuMove.fromX = -1;
            cpuMove.fromY = -1;

            cpuMove.toX = move.x_;
            cpuMove.toY = move.y_;

            cpuMove.pieceType = type;

            cpuMove.score = 0;

            moveList.push_back(cpuMove);
        }
    }
}

CpuMove MiniShogiCpu::FindBestMove(
    const std::vector<CpuMove>& moveList,
    const MiniShogiBoard& board)
{
    CpuMove bestMove = moveList.front();

    int bestScore = -100000000;

    for (auto move : moveList)
    {
        move.score = Evaluate(board, move);

        if (move.score > bestScore)
        {
            bestScore = move.score;
            bestMove = move;
        }
    }

    return bestMove;
}

int MiniShogiCpu::Evaluate(
    const MiniShogiBoard& board,
    const CpuMove& move)
{
    int score = 0;

    MiniShogiBoard nextBoard = board;

    ApplyMove(
        nextBoard,
        move);

    score += EvaluateCapture(board, move);

    score += EvaluatePromote(
        board,
        move,
        MiniShogiRule());

    score += EvaluatePosition(move);

    MiniShogiRule rule;

    if (rule.IsCheck(nextBoard, true))
    {
        score += CHECK_BONUS;
    }

    if (rule.IsCheck(nextBoard, false))
    {
        score -= 100000;
    }

    score += rand() % RANDOM_WIDTH;

    return score;
}

int MiniShogiCpu::EvaluatePiece(
    PieceType piece) const
{
    switch (piece)
    {
    case PieceType::OU:
        return VALUE_OU;

    case PieceType::KIN:
        return VALUE_KIN;

    case PieceType::GIN:
        return VALUE_GIN;

    case PieceType::KAKUGYO:
        return VALUE_KAKU;

    case PieceType::HISHA:
        return VALUE_HISHA;

    case PieceType::FU:
        return VALUE_FU;

    default:
        return 0;
    }
}

int MiniShogiCpu::EvaluateCapture(
    const MiniShogiBoard& board,
    const CpuMove& move) const
{
    if (!board.IsExistPiece(
        move.toX,
        move.toY))
    {
        return 0;
    }

    const Piece& piece =
        board.GetPiece(
            move.toX,
            move.toY);

    if (piece.isPlayer_ == CPU_SIDE)
    {
        return -10000;
    }

    return EvaluatePiece(
        piece.type_);
}

int MiniShogiCpu::EvaluatePromote(
    const MiniShogiBoard& board,
    const CpuMove& move,
    const MiniShogiRule& rule) const
{
    if (!move.isPromote)
    {
        return 0;
    }

    return PROMOTE_BONUS;
}

int MiniShogiCpu::EvaluatePosition(
    const CpuMove& move) const
{
    int score = 0;

    //---------------------------------
    // ëäéËêwÇ…ãﬂÇ√Ç≠ÇŸÇ«â¡ì_
    //---------------------------------

    score += move.toY * 20;

    //---------------------------------
    // ê^ÇÒíÜÇ…äÒÇÈÇŸÇ«â¡ì_
    //---------------------------------

    score += (2 - abs(move.toX - 2)) * 10;

    return score;
}

bool MiniShogiCpu::IsCpuPiece(
    const Piece& piece) const
{
    return
        piece.type_ != PieceType::NONE &&
        piece.isPlayer_ == CPU_SIDE;
}

bool MiniShogiCpu::IsInsideBoard(
    int x,
    int y) const
{
    return x >= 0 &&
        x < 5 &&
        y >= 0 &&
        y < 5;
}

void MiniShogiCpu::ApplyMove(
    MiniShogiBoard& board,
    const CpuMove& move)
{
    //----------------------------------
    // éùÇøãÓÇë≈Ç¬
    //----------------------------------
    if (move.isDrop)
    {
        Piece piece;

        piece.type_ = move.pieceType;
        piece.isPlayer_ = CPU_SIDE;
        piece.isPromote_ = false;

        board.SetPiece(
            move.toX,
            move.toY,
            piece);

        return;
    }

    //----------------------------------
    // ãÓÇìÆÇ©Ç∑
    //----------------------------------

    Piece piece =
        board.GetPiece(
            move.fromX,
            move.fromY);

    if (move.isPromote)
    {
        piece.isPromote_ = true;
    }

    board.SetPiece(
        move.toX,
        move.toY,
        piece);

    board.RemovePiece(
        move.fromX,
        move.fromY);
}