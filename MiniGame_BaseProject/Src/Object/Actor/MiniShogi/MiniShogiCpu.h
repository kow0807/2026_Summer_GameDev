#pragma once
#include <vector>

#include "MiniShogiBoard.h"
#include "MiniShogiRule.h"
#include "Hand.h"

struct CpuMove
{
    bool isDrop;          // 打つ手か
    bool isPromote;       // 成るか

    int fromX;
    int fromY;

    int toX;
    int toY;

    PieceType pieceType;  // 打つ駒

    int score;
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
        const Hand& hand,
        const MiniShogiRule& rule);

private:

    //----------------------------------
    // 合法手生成
    //----------------------------------

    void CreateMoveList(
        const MiniShogiBoard& board,
        const Hand& hand,
        const MiniShogiRule& rule,
        std::vector<CpuMove>& moveList);

    void AddBoardMove(
        const MiniShogiBoard& board,
        const MiniShogiRule& rule,
        int fromX,
        int fromY,
        std::vector<CpuMove>& moveList);

    void AddDropMove(
        const MiniShogiBoard& board,
        const Hand& hand,
        const MiniShogiRule& rule,
        std::vector<CpuMove>& moveList);

    //----------------------------------
    // 評価
    //----------------------------------

    int Evaluate(
        const MiniShogiBoard& board,
        const CpuMove& move);

    int EvaluatePiece(
        PieceType piece) const;

    int EvaluateCapture(
        const MiniShogiBoard& board,
        const CpuMove& move) const;

    int EvaluatePromote(
        const MiniShogiBoard& board,
        const CpuMove& move,
        const MiniShogiRule& rule) const;

    int EvaluatePosition(
        const CpuMove& move) const;

    //----------------------------------
    // 盤面シミュレーション
    //----------------------------------

    void ApplyMove(
        MiniShogiBoard& board,
        const CpuMove& move);

    //----------------------------------
    // 補助
    //----------------------------------

    bool IsCpuPiece(
        const Piece& piece) const;

    bool IsInsideBoard(
        int x,
        int y) const;

    CpuMove FindBestMove(
        const std::vector<CpuMove>& moveList,
        const MiniShogiBoard& board);


    //--------------------------------
   // CPU側
   //--------------------------------
    static constexpr bool CPU_SIDE = false;

    //--------------------------------
    // 評価値
    //--------------------------------
    static constexpr int VALUE_OU = 100000;
    static constexpr int VALUE_KIN = 700;
    static constexpr int VALUE_GIN = 500;
    static constexpr int VALUE_KAKU = 900;
    static constexpr int VALUE_HISHA = 1000;
    static constexpr int VALUE_FU = 100;

    //--------------------------------
    // 成駒ボーナス
    //--------------------------------
    static constexpr int PROMOTE_BONUS = 300;

    //--------------------------------
    // 王手
    //--------------------------------
    static constexpr int CHECK_BONUS = 800;

    //--------------------------------
    // 乱数幅
    //--------------------------------
    static constexpr int RANDOM_WIDTH = 15;
};

