#pragma once

#include <vector>

#include "MiniShogiBoard.h"
#include "MiniShogiRule.h"
#include "Hand.h"


struct CpuMove
{
    bool isDrop = false;
    bool isPromote = false;

    int fromX = -1;
    int fromY = -1;

    int toX = -1;
    int toY = -1;

    PieceType pieceType = PieceType::NONE;

    int score = 0;
};


class MiniShogiCpu
{
public:

    MiniShogiCpu();
    ~MiniShogiCpu();

    //----------------------------------
    // 思考
    //----------------------------------

    CpuMove Think(
        const MiniShogiBoard& board,
        const Hand& cpuHand,
        const Hand& playerHand,
        const MiniShogiRule& rule);

private:

    //----------------------------------
    // 合法手生成
    //----------------------------------

    void CreateMoveList(
        const MiniShogiBoard& board,
        const Hand& hand,
        const MiniShogiRule& rule,
        bool side,
        std::vector<CpuMove>& moveList) const;

    void AddBoardMove(
        const MiniShogiBoard& board,
        const MiniShogiRule& rule,
        bool side,
        int fromX,
        int fromY,
        std::vector<CpuMove>& moveList) const;

    void AddDropMove(
        const MiniShogiBoard& board,
        const Hand& hand,
        const MiniShogiRule& rule,
        bool side,
        std::vector<CpuMove>& moveList) const;

    //----------------------------------
    // Minimax
    //----------------------------------

    int Minimax(
        const MiniShogiBoard& board,
        const Hand& cpuHand,
        const Hand& playerHand,
        const MiniShogiRule& rule,
        int depth,
        bool cpuTurn,
        int alpha,
        int beta);

    //----------------------------------
    // 評価
    //----------------------------------

    int EvaluateBoard(
        const MiniShogiBoard& board,
        const Hand& cpuHand,
        const Hand& playerHand,
        const MiniShogiRule& rule) const;

    int EvaluatePiece(
        PieceType piece) const;

    int EvaluatePromotedPiece(
        PieceType piece) const;

    int EvaluatePosition(
        const Piece& piece,
        int x,
        int y) const;

    //----------------------------------
    // シミュレーション
    //----------------------------------

    void ApplyMoveSimulation(
        MiniShogiBoard& board,
        Hand& hand,
        const CpuMove& move,
        bool side) const;

    //----------------------------------
    // 手順並べ替え
    //----------------------------------

    void SortMoveList(
        const MiniShogiBoard& board,
        std::vector<CpuMove>& moveList) const;

    int EvaluateMoveOrder(
        const MiniShogiBoard& board,
        const CpuMove& move) const;

    //----------------------------------
    // 補助
    //----------------------------------

    bool IsSidePiece(
        const Piece& piece,
        bool side) const;

    bool IsInsideBoard(
        int x,
        int y) const;

    //----------------------------------
    // CPU側
    //----------------------------------

    static constexpr bool CPU_SIDE = false;
    static constexpr bool PLAYER_SIDE = true;

    //----------------------------------
    // 盤面サイズ
    //----------------------------------

    static constexpr int BOARD_SIZE = 5;

    //----------------------------------
    // 探索深度
    //----------------------------------

    static constexpr int SEARCH_DEPTH = 3;

    //----------------------------------
    // 評価値
    //----------------------------------

    static constexpr int VALUE_OU = 100000;
    static constexpr int VALUE_KIN = 650;
    static constexpr int VALUE_GIN = 550;
    static constexpr int VALUE_KAKU = 850;
    static constexpr int VALUE_HISHA = 950;
    static constexpr int VALUE_FU = 100;

    //----------------------------------
    // 王手
    //----------------------------------

    static constexpr int CHECK_BONUS = 400;

    //----------------------------------
    // 詰み
    //----------------------------------

    static constexpr int CHECKMATE_SCORE = 10000000;

    //----------------------------------
    // 位置評価
    //----------------------------------

    static constexpr int ADVANCE_BONUS = 10;
    static constexpr int CENTER_BONUS = 5;
};