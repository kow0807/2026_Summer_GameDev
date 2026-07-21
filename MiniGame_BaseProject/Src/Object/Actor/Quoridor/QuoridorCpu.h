#pragma once

#include <utility>
#include <vector>

class QuoridorBoard;
struct Player;

class QuoridorCpu
{
public:
    static constexpr int BOARD_SIZE = 9;

    struct CpuAction
    {
        bool isWall = false;
        int x = 0;
        int y = 0;
        bool isVertical = false;
    };

    struct BOARDSTATE
    {
        int playerX[2] = {};
        int playerY[2] = {};
        int remainingWalls[2] = {};

        // [x][y]
        bool verticalWalls[BOARD_SIZE - 1][BOARD_SIZE] = {};
        bool horizontalWalls[BOARD_SIZE][BOARD_SIZE - 1] = {};
    };

public:
    bool FindBestAction(
        const QuoridorBoard& board,
        const Player players[2],
        CpuAction& outAction,
        int searchDepth = 3) const;

private:
    BOARDSTATE CreateState(
        const QuoridorBoard& board,
        const Player players[2]) const;

    bool CanMove(
        const BOARDSTATE& state,
        int fromX,
        int fromY,
        int dirX,
        int dirY) const;

    std::vector<std::pair<int, int>> GetMoveCandidates(
        const BOARDSTATE& state,
        int playerIndex) const;

    int GetShortestDistance(
        const BOARDSTATE& state,
        int startX,
        int startY,
        int goalY) const;

    std::vector<std::pair<int, int>> GetShortestPathCells(
        const BOARDSTATE& state,
        int startX,
        int startY,
        int goalY) const;

    bool CanPlaceWall(
        const BOARDSTATE& state,
        int x,
        int y,
        bool isVertical) const;

    bool CanPlaceWallWithReachability(
        const BOARDSTATE& state,
        int x,
        int y,
        bool isVertical) const;

    bool ApplyMoveUnchecked(
        BOARDSTATE& state,
        int playerIndex,
        int toX,
        int toY) const;

    void ApplyWallUnchecked(
        BOARDSTATE& state,
        int playerIndex,
        int x,
        int y,
        bool isVertical) const;

    void ApplyActionUnchecked(
        BOARDSTATE& state,
        int playerIndex,
        const CpuAction& action) const;

    bool IsGoal(
        const BOARDSTATE& state,
        int playerIndex) const;

    int Evaluate(
        const BOARDSTATE& state) const;

    std::vector<CpuAction> GenerateMoves(
        const BOARDSTATE& state,
        int playerIndex,
        bool useWall,
        int maxWallActions) const;

    int Minimax(
        const BOARDSTATE& state,
        int depth,
        int currentPlayerIndex,
        int alpha,
        int beta,
        bool useWall) const;
};