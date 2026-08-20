#include "MiniShogiCpu.h"

#include <algorithm>
#include <limits>
#include <cstdlib>

MiniShogiCpu::MiniShogiCpu()
{
}

MiniShogiCpu::~MiniShogiCpu()
{
}

CpuMove MiniShogiCpu::Think(
    const MiniShogiBoard& board,
    const Hand& cpuHand,
    const Hand& playerHand,
    const MiniShogiRule& rule)
{
    std::vector<CpuMove> moveList;

    //----------------------------------
    // CPUの合法手を生成
    //----------------------------------

    CreateMoveList(
        board,
        cpuHand,
        rule,
        CPU_SIDE,
        moveList);

    //----------------------------------
    // 指せる手がない
    //----------------------------------

    if (moveList.empty())
    {
        return CpuMove{};
    }

    //----------------------------------
    // αβ枝刈りしやすいように
    // 強そうな手を先に調べる
    //----------------------------------

    SortMoveList(
        board,
        moveList);

    CpuMove bestMove =
        moveList.front();

    int bestScore =
        -CHECKMATE_SCORE;

    int alpha =
        -CHECKMATE_SCORE;

    const int beta =
        CHECKMATE_SCORE;

    //----------------------------------
    // 全候補を探索
    //----------------------------------

    for (CpuMove move : moveList)
    {
        MiniShogiBoard nextBoard =
            board;

        Hand nextCpuHand =
            cpuHand;

        Hand nextPlayerHand =
            playerHand;

        //----------------------------------
        // CPUの手を仮実行
        //----------------------------------

        ApplyMoveSimulation(
            nextBoard,
            nextCpuHand,
            move,
            CPU_SIDE);

        //----------------------------------
        // 次はプレイヤーの番
        //----------------------------------

        int score =
            Minimax(
                nextBoard,
                nextCpuHand,
                nextPlayerHand,
                rule,
                SEARCH_DEPTH - 1,
                false,
                alpha,
                beta);

        move.score = score;

        //----------------------------------
        // 最良手更新
        //----------------------------------

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        alpha =
            std::max(
                alpha,
                bestScore);
    }

    bestMove.score =
        bestScore;

    return bestMove;
}

void MiniShogiCpu::CreateMoveList(
    const MiniShogiBoard& board,
    const Hand& hand,
    const MiniShogiRule& rule,
    bool side,
    std::vector<CpuMove>& moveList) const
{
    moveList.clear();

    //----------------------------------
    // 盤上の駒
    //----------------------------------

    for (int y = 0;
        y < BOARD_SIZE;
        ++y)
    {
        for (int x = 0;
            x < BOARD_SIZE;
            ++x)
        {
            if (!board.IsExistPiece(
                x,
                y))
            {
                continue;
            }

            const Piece& piece =
                board.GetPiece(
                    x,
                    y);

            //----------------------------------
            // 指定された側の駒だけ
            //----------------------------------

            if (!IsSidePiece(
                piece,
                side))
            {
                continue;
            }

            AddBoardMove(
                board,
                rule,
                side,
                x,
                y,
                moveList);
        }
    }

    //----------------------------------
    // 持ち駒
    //----------------------------------

    AddDropMove(
        board,
        hand,
        rule,
        side,
        moveList);
}

void MiniShogiCpu::AddBoardMove(
    const MiniShogiBoard& board,
    const MiniShogiRule& rule,
    bool side,
    int fromX,
    int fromY,
    std::vector<CpuMove>& moveList) const
{
    if (!board.IsExistPiece(
        fromX,
        fromY))
    {
        return;
    }

    const Piece& piece =
        board.GetPiece(
            fromX,
            fromY);

    if (!IsSidePiece(
        piece,
        side))
    {
        return;
    }

    //----------------------------------
    // 合法手取得
    //----------------------------------

    const std::vector<MoveData> list =
        rule.GetLegalMoveList(
            board,
            fromX,
            fromY);

    for (const MoveData& move : list)
    {
        //----------------------------------
        // 王そのものを取る手は生成しない
        //----------------------------------

        if (board.IsExistPiece(
            move.x_,
            move.y_))
        {
            const Piece& target =
                board.GetPiece(
                    move.x_,
                    move.y_);

            if (target.type_ ==
                PieceType::OU)
            {
                continue;
            }
        }

        //----------------------------------
        // 強制成り
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

            moveList.push_back(
                cpuMove);

            continue;
        }

        //----------------------------------
        // 成らない
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

        moveList.push_back(
            normalMove);

        //----------------------------------
        // 任意成り
        //----------------------------------

        if (rule.CanPromote(
            piece,
            fromY,
            move.y_))
        {
            CpuMove promoteMove =
                normalMove;

            promoteMove.isPromote =
                true;

            moveList.push_back(
                promoteMove);
        }
    }
}

void MiniShogiCpu::AddDropMove(
    const MiniShogiBoard& board,
    const Hand& hand,
    const MiniShogiRule& rule,
    bool side,
    std::vector<CpuMove>& moveList) const
{
    //----------------------------------
    // 同じ種類の駒を2枚持っていても
    // 同じ打ち手を何度も生成しない
    //----------------------------------

    std::vector<PieceType> processedTypes;

    for (int i = 0;
        i < hand.GetPieceCount();
        ++i)
    {
        PieceType type =
            hand.GetPiece(i).type_;

        //----------------------------------
        // すでに処理した種類なら飛ばす
        //----------------------------------

        if (std::find(
            processedTypes.begin(),
            processedTypes.end(),
            type) != processedTypes.end())
        {
            continue;
        }

        processedTypes.push_back(
            type);

        //----------------------------------
        // 合法な駒打ち
        //----------------------------------

        const auto list =
            rule.GetLegalDropList(
                board,
                type,
                side);

        for (const auto& move : list)
        {
            CpuMove cpuMove{};

            cpuMove.isDrop =
                true;

            cpuMove.isPromote =
                false;

            cpuMove.fromX =
                -1;

            cpuMove.fromY =
                -1;

            cpuMove.toX =
                move.x_;

            cpuMove.toY =
                move.y_;

            cpuMove.pieceType =
                type;

            moveList.push_back(
                cpuMove);
        }
    }
}

int MiniShogiCpu::Minimax(
    const MiniShogiBoard& board,
    const Hand& cpuHand,
    const Hand& playerHand,
    const MiniShogiRule& rule,
    int depth,
    bool cpuTurn,
    int alpha,
    int beta)
{
    //----------------------------------
    // 現在手番
    //----------------------------------

    const bool currentSide =
        cpuTurn
        ? CPU_SIDE
        : PLAYER_SIDE;

    const Hand& currentHand =
        cpuTurn
        ? cpuHand
        : playerHand;

    //----------------------------------
    // 合法手が存在するか
    //----------------------------------

    const bool hasLegalMove =
        rule.HasAnyLegalMove(
            board,
            currentHand,
            currentSide);

    //----------------------------------
    // 手がない = ゲーム終了
    //----------------------------------

    if (!hasLegalMove)
    {
        //----------------------------------
        // CPU手番で手がない
        // CPUの負け
        //----------------------------------

        if (cpuTurn)
        {
            return
                -CHECKMATE_SCORE
                - depth;
        }

        //----------------------------------
        // プレイヤー手番で手がない
        // CPUの勝ち
        //----------------------------------

        return
            CHECKMATE_SCORE
            + depth;
    }

    //----------------------------------
    // 探索深度終了
    //----------------------------------

    if (depth <= 0)
    {
        return EvaluateBoard(
            board,
            cpuHand,
            playerHand,
            rule);
    }

    //----------------------------------
    // 合法手生成
    //----------------------------------

    std::vector<CpuMove> moveList;

    CreateMoveList(
        board,
        currentHand,
        rule,
        currentSide,
        moveList);

    if (moveList.empty())
    {
        if (cpuTurn)
        {
            return
                -CHECKMATE_SCORE
                - depth;
        }

        return
            CHECKMATE_SCORE
            + depth;
    }

    //----------------------------------
    // 良さそうな手を先に探索
    //----------------------------------

    SortMoveList(
        board,
        moveList);

    //----------------------------------
    // CPU
    // 最大化
    //----------------------------------

    if (cpuTurn)
    {
        int bestScore =
            -CHECKMATE_SCORE;

        for (const CpuMove& move :
            moveList)
        {
            MiniShogiBoard nextBoard =
                board;

            Hand nextCpuHand =
                cpuHand;

            Hand nextPlayerHand =
                playerHand;

            ApplyMoveSimulation(
                nextBoard,
                nextCpuHand,
                move,
                CPU_SIDE);

            int score =
                Minimax(
                    nextBoard,
                    nextCpuHand,
                    nextPlayerHand,
                    rule,
                    depth - 1,
                    false,
                    alpha,
                    beta);

            bestScore =
                std::max(
                    bestScore,
                    score);

            alpha =
                std::max(
                    alpha,
                    bestScore);

            //----------------------------------
            // βカット
            //----------------------------------

            if (alpha >= beta)
            {
                break;
            }
        }

        return bestScore;
    }

    //----------------------------------
    // PLAYER
    // 最小化
    //----------------------------------

    int bestScore =
        CHECKMATE_SCORE;

    for (const CpuMove& move :
        moveList)
    {
        MiniShogiBoard nextBoard =
            board;

        Hand nextCpuHand =
            cpuHand;

        Hand nextPlayerHand =
            playerHand;

        ApplyMoveSimulation(
            nextBoard,
            nextPlayerHand,
            move,
            PLAYER_SIDE);

        int score =
            Minimax(
                nextBoard,
                nextCpuHand,
                nextPlayerHand,
                rule,
                depth - 1,
                true,
                alpha,
                beta);

        bestScore =
            std::min(
                bestScore,
                score);

        beta =
            std::min(
                beta,
                bestScore);

        //----------------------------------
        // αカット
        //----------------------------------

        if (alpha >= beta)
        {
            break;
        }
    }

    return bestScore;
}

int MiniShogiCpu::EvaluateBoard(
    const MiniShogiBoard& board,
    const Hand& cpuHand,
    const Hand& playerHand,
    const MiniShogiRule& rule) const
{
    int score = 0;

    //----------------------------------
    // 盤上の駒
    //----------------------------------

    for (int y = 0;
        y < BOARD_SIZE;
        ++y)
    {
        for (int x = 0;
            x < BOARD_SIZE;
            ++x)
        {
            if (!board.IsExistPiece(
                x,
                y))
            {
                continue;
            }

            const Piece& piece =
                board.GetPiece(
                    x,
                    y);

            int value =
                EvaluatePiece(
                    piece.type_);

            //----------------------------------
            // 成駒
            //----------------------------------

            if (piece.isPromote_)
            {
                value +=
                    EvaluatePromotedPiece(
                        piece.type_);
            }

            //----------------------------------
            // CPU
            //----------------------------------

            if (piece.isPlayer_ ==
                CPU_SIDE)
            {
                score += value;

                score +=
                    EvaluatePosition(
                        piece,
                        x,
                        y);
            }
            //----------------------------------
            // PLAYER
            //----------------------------------
            else
            {
                score -= value;

                score -=
                    EvaluatePosition(
                        piece,
                        x,
                        y);
            }
        }
    }

    //----------------------------------
 // CPU持ち駒
 //----------------------------------

    for (int i = 0;
        i < cpuHand.GetPieceCount();
        ++i)
    {
        const HandPiece& piece =
            cpuHand.GetPiece(i);

        score +=
            EvaluatePiece(
                piece.type_);
    }

    //----------------------------------
    // プレイヤー持ち駒
    //----------------------------------

    for (int i = 0;
        i < playerHand.GetPieceCount();
        ++i)
    {
        const HandPiece& piece =
            playerHand.GetPiece(i);

        score -=
            EvaluatePiece(
                piece.type_);
    }

    //----------------------------------
    // プレイヤーが王手
    // CPUに有利
    //----------------------------------

    if (rule.IsCheck(
        board,
        PLAYER_SIDE))
    {
        score +=
            CHECK_BONUS;
    }

    //----------------------------------
    // CPUが王手
    // CPUに不利
    //----------------------------------

    if (rule.IsCheck(
        board,
        CPU_SIDE))
    {
        score -=
            CHECK_BONUS;
    }

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

int MiniShogiCpu::EvaluatePromotedPiece(
    PieceType piece) const
{
    switch (piece)
    {
        //----------------------------------
        // と金
        //----------------------------------

    case PieceType::FU:
        return 450;

        //----------------------------------
        // 成銀
        //----------------------------------

    case PieceType::GIN:
        return 100;

        //----------------------------------
        // 馬
        //----------------------------------

    case PieceType::KAKUGYO:
        return 300;

        //----------------------------------
        // 龍
        //----------------------------------

    case PieceType::HISHA:
        return 350;

    default:
        return 0;
    }
}

int MiniShogiCpu::EvaluatePosition(
    const Piece& piece,
    int x,
    int y) const
{
    int score = 0;

    //----------------------------------
    // 前進評価
    //----------------------------------

    if (piece.isPlayer_ ==
        CPU_SIDE)
    {
        //----------------------------------
        // CPUはyが大きいほど敵陣
        //----------------------------------

        score +=
            y *
            ADVANCE_BONUS;
    }
    else
    {
        //----------------------------------
        // プレイヤーはyが小さいほど敵陣
        //----------------------------------

        score +=
            (BOARD_SIZE - 1 - y) *
            ADVANCE_BONUS;
    }

    //----------------------------------
    // 中央評価
    //----------------------------------

    const int centerX =
        BOARD_SIZE / 2;

    const int distance =
        std::abs(
            x - centerX);

    score +=
        (centerX - distance) *
        CENTER_BONUS;

    return score;
}

void MiniShogiCpu::ApplyMoveSimulation(
    MiniShogiBoard& board,
    Hand& hand,
    const CpuMove& move,
    bool side) const
{
    //----------------------------------
    // 持ち駒を打つ
    //----------------------------------

    if (move.isDrop)
    {
        Piece piece{};

        piece.type_ =
            move.pieceType;

        piece.isPlayer_ =
            side;

        piece.isPromote_ =
            false;

        board.SetPiece(
            move.toX,
            move.toY,
            piece);

        //----------------------------------
        // 持ち駒から削除
        //----------------------------------

        hand.RemovePiece(
            move.pieceType);

        return;
    }

    //----------------------------------
    // 移動する駒
    //----------------------------------

    Piece movePiece =
        board.GetPiece(
            move.fromX,
            move.fromY);

    //----------------------------------
    // 相手駒を取る
    //----------------------------------

    if (board.IsExistPiece(
        move.toX,
        move.toY))
    {
        Piece capturePiece =
            board.GetPiece(
                move.toX,
                move.toY);

        //----------------------------------
        // 王は取らない
        //----------------------------------

        if (capturePiece.type_ !=
            PieceType::OU)
        {
            //----------------------------------
            // 持ち駒になったら成り解除
            //----------------------------------

            capturePiece.isPromote_ =
                false;

            capturePiece.isPlayer_ =
                side;

            hand.AddPiece(
                capturePiece.type_);
        }
    }

    //----------------------------------
    // 成る
    //----------------------------------

    if (move.isPromote)
    {
        movePiece.isPromote_ =
            true;
    }

    movePiece.isPlayer_ =
        side;

    //----------------------------------
    // 移動
    //----------------------------------

    board.SetPiece(
        move.toX,
        move.toY,
        movePiece);

    board.RemovePiece(
        move.fromX,
        move.fromY);
}

void MiniShogiCpu::SortMoveList(
    const MiniShogiBoard& board,
    std::vector<CpuMove>& moveList) const
{
    std::sort(
        moveList.begin(),
        moveList.end(),
        [this, &board]
        (
            const CpuMove& a,
            const CpuMove& b
            )
        {
            return
                EvaluateMoveOrder(
                    board,
                    a)
                >
                EvaluateMoveOrder(
                    board,
                    b);
        });
}

int MiniShogiCpu::EvaluateMoveOrder(
    const MiniShogiBoard& board,
    const CpuMove& move) const
{
    int score = 0;

    //----------------------------------
    // 駒取りを優先
    //----------------------------------

    if (IsInsideBoard(
        move.toX,
        move.toY) &&
        board.IsExistPiece(
            move.toX,
            move.toY))
    {
        const Piece& capturePiece =
            board.GetPiece(
                move.toX,
                move.toY);

        score +=
            EvaluatePiece(
                capturePiece.type_);
    }

    //----------------------------------
    // 成りを優先
    //----------------------------------

    if (move.isPromote)
    {
        score +=
            500;
    }

    //----------------------------------
    // 打ち
    //----------------------------------

    if (move.isDrop)
    {
        score +=
            50;
    }

    return score;
}

bool MiniShogiCpu::IsSidePiece(
    const Piece& piece,
    bool side) const
{
    return
        piece.type_ !=
        PieceType::NONE
        &&
        piece.isPlayer_ ==
        side;
}

bool MiniShogiCpu::IsInsideBoard(
    int x,
    int y) const
{
    return
        x >= 0 &&
        x < BOARD_SIZE &&
        y >= 0 &&
        y < BOARD_SIZE;
}