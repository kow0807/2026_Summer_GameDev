#define NOMINMAX
#include <Windows.h>

#include <DxLib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

#include "QuoridorBoard.h"
#include "QuoridorCpu.h"

namespace
{
    constexpr int UNREACHABLE_DISTANCE = 999;
    constexpr int WIN_SCORE = 100000;
    constexpr int LOSE_SCORE = -100000;

    constexpr int ROOT_MAX_WALL_ACTIONS = 32;
    constexpr int SEARCH_MAX_WALL_ACTIONS = 16;
}

QuoridorCpu::BOARDSTATE QuoridorCpu::CreateState(
    const QuoridorBoard& board,
    const Player players[2]) const
{
    BOARDSTATE state{};

    for (int i = 0; i < 2; ++i)
    {
        state.playerX[i] = players[i].x_;
        state.playerY[i] = players[i].y_;
        state.remainingWalls[i] = players[i].remainingWalls_;
    }

    for (int x = 0; x < BOARD_SIZE - 1; ++x)
    {
        for (int y = 0; y < BOARD_SIZE; ++y)
        {
            state.verticalWalls[x][y] =
                board.GetVerticalWall(x, y);
        }
    }

    for (int x = 0; x < BOARD_SIZE; ++x)
    {
        for (int y = 0; y < BOARD_SIZE - 1; ++y)
        {
            state.horizontalWalls[x][y] =
                board.GetHorizontalWall(x, y);
        }
    }

    return state;
}

bool QuoridorCpu::CanMove(
    const BOARDSTATE& state,
    int fromX,
    int fromY,
    int dirX,
    int dirY) const
{
    if (fromX < 0 || fromX >= BOARD_SIZE ||
        fromY < 0 || fromY >= BOARD_SIZE)
    {
        return false;
    }

    if (std::abs(dirX) + std::abs(dirY) != 1)
    {
        return false;
    }

    const int toX = fromX + dirX;
    const int toY = fromY + dirY;

    if (toX < 0 || toX >= BOARD_SIZE ||
        toY < 0 || toY >= BOARD_SIZE)
    {
        return false;
    }

    if (dirX == 1)
    {
        if (state.verticalWalls[fromX][fromY])
        {
            return false;
        }

        if (fromY > 0 &&
            state.verticalWalls[fromX][fromY - 1])
        {
            return false;
        }
    }
    else if (dirX == -1)
    {
        if (state.verticalWalls[fromX - 1][fromY])
        {
            return false;
        }

        if (fromY > 0 &&
            state.verticalWalls[fromX - 1][fromY - 1])
        {
            return false;
        }
    }
    else if (dirY == 1)
    {
        if (state.horizontalWalls[fromX][fromY])
        {
            return false;
        }

        if (fromX > 0 &&
            state.horizontalWalls[fromX - 1][fromY])
        {
            return false;
        }
    }
    else
    {
        if (state.horizontalWalls[fromX][fromY - 1])
        {
            return false;
        }

        if (fromX > 0 &&
            state.horizontalWalls[fromX - 1][fromY - 1])
        {
            return false;
        }
    }

    return true;
}

std::vector<std::pair<int, int>>
QuoridorCpu::GetMoveCandidates(
    const BOARDSTATE& state,
    int playerIndex) const
{
    std::vector<std::pair<int, int>> candidates;

    if (playerIndex < 0 || playerIndex >= 2)
    {
        return candidates;
    }

    const int opponentIndex = 1 - playerIndex;

    const int playerX = state.playerX[playerIndex];
    const int playerY = state.playerY[playerIndex];
    const int opponentX = state.playerX[opponentIndex];
    const int opponentY = state.playerY[opponentIndex];

    // Pythonî≈Ç∆ìØÇ∂íTçıèá
    constexpr int DIRECTIONS[4][2] =
    {
        { 0, -1 },
        { 1,  0 },
        {-1,  0 },
        { 0,  1 }
    };

    const auto addCandidate =
        [&candidates](int x, int y)
        {
            for (const auto& candidate : candidates)
            {
                if (candidate.first == x &&
                    candidate.second == y)
                {
                    return;
                }
            }

            candidates.push_back({ x, y });
        };

    for (const auto& direction : DIRECTIONS)
    {
        const int dirX = direction[0];
        const int dirY = direction[1];

        if (!CanMove(
            state,
            playerX,
            playerY,
            dirX,
            dirY))
        {
            continue;
        }

        const int nextX = playerX + dirX;
        const int nextY = playerY + dirY;

        if (nextX != opponentX ||
            nextY != opponentY)
        {
            addCandidate(nextX, nextY);
            continue;
        }

        // ëäéËÇÃå„ÇÎÇ÷íºê¸ÉWÉÉÉìÉv
        if (CanMove(
            state,
            opponentX,
            opponentY,
            dirX,
            dirY))
        {
            addCandidate(
                opponentX + dirX,
                opponentY + dirY);

            continue;
        }

        // íºê¸ÉWÉÉÉìÉvïsâ¬Ç»ÇÁéŒÇﬂà⁄ìÆ
        const int sideDirections[2][2] =
        {
            {-dirY, dirX},
            { dirY,-dirX}
        };

        for (const auto& side : sideDirections)
        {
            if (!CanMove(
                state,
                opponentX,
                opponentY,
                side[0],
                side[1]))
            {
                continue;
            }

            addCandidate(
                opponentX + side[0],
                opponentY + side[1]);
        }
    }

    return candidates;
}

int QuoridorCpu::GetShortestDistance(
    const BOARDSTATE& state,
    int startX,
    int startY,
    int goalY) const
{
    if (startX < 0 || startX >= BOARD_SIZE ||
        startY < 0 || startY >= BOARD_SIZE ||
        goalY < 0 || goalY >= BOARD_SIZE)
    {
        return UNREACHABLE_DISTANCE;
    }

    if (startY == goalY)
    {
        return 0;
    }

    struct SearchNode
    {
        int x;
        int y;
        int distance;
    };

    bool visited[BOARD_SIZE][BOARD_SIZE] = {};
    std::queue<SearchNode> searchQueue;

    visited[startX][startY] = true;
    searchQueue.push({ startX, startY, 0 });

    constexpr int DIRECTIONS[4][2] =
    {
        { 1,  0 },
        {-1,  0 },
        { 0,  1 },
        { 0, -1 }
    };

    while (!searchQueue.empty())
    {
        const SearchNode current =
            searchQueue.front();

        searchQueue.pop();

        for (const auto& direction : DIRECTIONS)
        {
            const int dirX = direction[0];
            const int dirY = direction[1];

            if (!CanMove(
                state,
                current.x,
                current.y,
                dirX,
                dirY))
            {
                continue;
            }

            const int nextX = current.x + dirX;
            const int nextY = current.y + dirY;

            if (visited[nextX][nextY])
            {
                continue;
            }

            if (nextY == goalY)
            {
                return current.distance + 1;
            }

            visited[nextX][nextY] = true;

            searchQueue.push({
                nextX,
                nextY,
                current.distance + 1
                });
        }
    }

    return UNREACHABLE_DISTANCE;
}

std::vector<std::pair<int, int>>
QuoridorCpu::GetShortestPathCells(
    const BOARDSTATE& state,
    int startX,
    int startY,
    int goalY) const
{
    std::vector<std::pair<int, int>> path;

    if (startX < 0 || startX >= BOARD_SIZE ||
        startY < 0 || startY >= BOARD_SIZE ||
        goalY < 0 || goalY >= BOARD_SIZE)
    {
        return path;
    }

    if (startY == goalY)
    {
        path.push_back({ startX, startY });
        return path;
    }

    struct SearchNode
    {
        int x;
        int y;
    };

    bool visited[BOARD_SIZE][BOARD_SIZE] = {};
    int parentX[BOARD_SIZE][BOARD_SIZE] = {};
    int parentY[BOARD_SIZE][BOARD_SIZE] = {};

    for (int x = 0; x < BOARD_SIZE; ++x)
    {
        for (int y = 0; y < BOARD_SIZE; ++y)
        {
            parentX[x][y] = -1;
            parentY[x][y] = -1;
        }
    }

    std::queue<SearchNode> searchQueue;
    searchQueue.push({ startX, startY });
    visited[startX][startY] = true;

    constexpr int DIRECTIONS[4][2] =
    {
        { 1,  0 },
        {-1,  0 },
        { 0,  1 },
        { 0, -1 }
    };

    int goalX = -1;

    while (!searchQueue.empty())
    {
        const SearchNode current =
            searchQueue.front();

        searchQueue.pop();

        if (current.y == goalY)
        {
            goalX = current.x;
            break;
        }

        for (const auto& direction : DIRECTIONS)
        {
            const int dirX = direction[0];
            const int dirY = direction[1];

            if (!CanMove(
                state,
                current.x,
                current.y,
                dirX,
                dirY))
            {
                continue;
            }

            const int nextX = current.x + dirX;
            const int nextY = current.y + dirY;

            if (visited[nextX][nextY])
            {
                continue;
            }

            visited[nextX][nextY] = true;
            parentX[nextX][nextY] = current.x;
            parentY[nextX][nextY] = current.y;

            searchQueue.push({ nextX, nextY });
        }
    }

    if (goalX < 0)
    {
        return path;
    }

    int currentX = goalX;
    int currentY = goalY;

    while (currentX != startX ||
        currentY != startY)
    {
        path.push_back({ currentX, currentY });

        const int previousX =
            parentX[currentX][currentY];

        const int previousY =
            parentY[currentX][currentY];

        if (previousX < 0 || previousY < 0)
        {
            path.clear();
            return path;
        }

        currentX = previousX;
        currentY = previousY;
    }

    path.push_back({ startX, startY });

    std::reverse(path.begin(), path.end());

    return path;
}

bool QuoridorCpu::CanPlaceWall(
    const BOARDSTATE& state,
    int x,
    int y,
    bool isVertical) const
{
    if (x < 0 || x >= BOARD_SIZE - 1 ||
        y < 0 || y >= BOARD_SIZE - 1)
    {
        return false;
    }

    if (isVertical)
    {
        if (state.verticalWalls[x][y])
        {
            return false;
        }

        if (y > 0 &&
            state.verticalWalls[x][y - 1])
        {
            return false;
        }

        if (y < BOARD_SIZE - 2 &&
            state.verticalWalls[x][y + 1])
        {
            return false;
        }

        if (state.horizontalWalls[x][y])
        {
            return false;
        }
    }
    else
    {
        if (state.horizontalWalls[x][y])
        {
            return false;
        }

        if (x > 0 &&
            state.horizontalWalls[x - 1][y])
        {
            return false;
        }

        if (x < BOARD_SIZE - 2 &&
            state.horizontalWalls[x + 1][y])
        {
            return false;
        }

        if (state.verticalWalls[x][y])
        {
            return false;
        }
    }

    return true;
}

bool QuoridorCpu::CanPlaceWallWithReachability(
    const BOARDSTATE& state,
    int x,
    int y,
    bool isVertical) const
{
    if (!CanPlaceWall(state, x, y, isVertical))
    {
        return false;
    }

    BOARDSTATE testState = state;

    if (isVertical)
    {
        testState.verticalWalls[x][y] = true;
    }
    else
    {
        testState.horizontalWalls[x][y] = true;
    }

    const int playerDistance =
        GetShortestDistance(
            testState,
            testState.playerX[0],
            testState.playerY[0],
            BOARD_SIZE - 1);

    if (playerDistance >= UNREACHABLE_DISTANCE)
    {
        return false;
    }

    const int cpuDistance =
        GetShortestDistance(
            testState,
            testState.playerX[1],
            testState.playerY[1],
            0);

    return cpuDistance < UNREACHABLE_DISTANCE;
}

bool QuoridorCpu::ApplyMoveUnchecked(
    BOARDSTATE& state,
    int playerIndex,
    int toX,
    int toY) const
{
    if (playerIndex < 0 || playerIndex >= 2)
    {
        return false;
    }

    state.playerX[playerIndex] = toX;
    state.playerY[playerIndex] = toY;

    return true;
}

void QuoridorCpu::ApplyWallUnchecked(
    BOARDSTATE& state,
    int playerIndex,
    int x,
    int y,
    bool isVertical) const
{
    if (isVertical)
    {
        state.verticalWalls[x][y] = true;
    }
    else
    {
        state.horizontalWalls[x][y] = true;
    }

    --state.remainingWalls[playerIndex];
}

void QuoridorCpu::ApplyActionUnchecked(
    BOARDSTATE& state,
    int playerIndex,
    const CpuAction& action) const
{
    if (action.isWall)
    {
        ApplyWallUnchecked(
            state,
            playerIndex,
            action.x,
            action.y,
            action.isVertical);

        return;
    }

    ApplyMoveUnchecked(
        state,
        playerIndex,
        action.x,
        action.y);
}

bool QuoridorCpu::IsGoal(
    const BOARDSTATE& state,
    int playerIndex) const
{
    if (playerIndex == 0)
    {
        return state.playerY[0] == BOARD_SIZE - 1;
    }

    if (playerIndex == 1)
    {
        return state.playerY[1] == 0;
    }

    return false;
}

int QuoridorCpu::Evaluate(
    const BOARDSTATE& state) const
{
    if (IsGoal(state, 1))
    {
        return WIN_SCORE;
    }

    if (IsGoal(state, 0))
    {
        return LOSE_SCORE;
    }

    const int playerDistance =
        GetShortestDistance(
            state,
            state.playerX[0],
            state.playerY[0],
            BOARD_SIZE - 1);

    const int cpuDistance =
        GetShortestDistance(
            state,
            state.playerX[1],
            state.playerY[1],
            0);

    double playerWeight = 0.8;

    if (playerDistance <= 2)
    {
        playerWeight = 4.5;
    }
    else if (playerDistance <= 4)
    {
        playerWeight = 2.5;
    }

    double score =
        (playerDistance * playerWeight) -
        (cpuDistance * 2.0);

    const int cpuMoveCount =
        static_cast<int>(
            GetMoveCandidates(state, 1).size());

    const int playerMoveCount =
        static_cast<int>(
            GetMoveCandidates(state, 0).size());

    score += cpuMoveCount * 0.4;
    score += (4 - playerMoveCount) * 0.15;

    // Pythonî≈Ç∆ìØíˆìxÇÃï]âøílÇêÆêîâª
    return static_cast<int>(score * 100.0);
}

std::vector<QuoridorCpu::CpuAction>
QuoridorCpu::GenerateMoves(
    const BOARDSTATE& state,
    int playerIndex,
    bool useWall,
    int maxWallActions) const
{
    std::vector<CpuAction> actions;

    if (playerIndex < 0 || playerIndex >= 2)
    {
        return actions;
    }

    const int goalY =
        (playerIndex == 0)
        ? BOARD_SIZE - 1
        : 0;

    auto moveCandidates =
        GetMoveCandidates(state, playerIndex);

    // ÉSÅ[ÉãÇ…ãﬂÇ¢à⁄ìÆÇ©ÇÁí≤Ç◊ÇƒÉøÉ¿é}ä†ÇËÇå¯Ç©ÇπÇÈ
    std::sort(
        moveCandidates.begin(),
        moveCandidates.end(),
        [this, &state, goalY](
            const std::pair<int, int>& left,
            const std::pair<int, int>& right)
        {
            const int leftDistance =
                GetShortestDistance(
                    state,
                    left.first,
                    left.second,
                    goalY);

            const int rightDistance =
                GetShortestDistance(
                    state,
                    right.first,
                    right.second,
                    goalY);

            return leftDistance < rightDistance;
        });

    for (const auto& move : moveCandidates)
    {
        CpuAction action{};
        action.isWall = false;
        action.x = move.first;
        action.y = move.second;

        actions.push_back(action);
    }

    if (!useWall ||
        maxWallActions <= 0 ||
        state.remainingWalls[playerIndex] <= 0)
    {
        return actions;
    }

    const int opponentIndex = 1 - playerIndex;

    const int playerDistance =
        GetShortestDistance(
            state,
            state.playerX[playerIndex],
            state.playerY[playerIndex],
            goalY);

    const int opponentGoalY =
        (opponentIndex == 0)
        ? BOARD_SIZE - 1
        : 0;

    const int opponentDistance =
        GetShortestDistance(
            state,
            state.playerX[opponentIndex],
            state.playerY[opponentIndex],
            opponentGoalY);

    // Pythonî≈wall_candidates()Ç∆ìØÇ∂ãóó£ç∑é}êÿÇË
    const int distanceDifference =
        playerDistance - opponentDistance;

    if (opponentDistance > 4 &&
        (distanceDifference < -2 ||
            distanceDifference > 3))
    {
        return actions;
    }

    const auto opponentPath =
        GetShortestPathCells(
            state,
            state.playerX[opponentIndex],
            state.playerY[opponentIndex],
            opponentGoalY);

    if (opponentPath.empty())
    {
        return actions;
    }

    struct ScoredWall
    {
        CpuAction action;
        int score;
    };

    std::vector<ScoredWall> wallActions;

    bool seen[BOARD_SIZE - 1][BOARD_SIZE - 1][2] = {};

    // ëäéËÇÃç≈íZåoòHé¸ï”ÇæÇØí≤Ç◊ÇÈ
    for (const auto& cell : opponentPath)
    {
        const int cellX = cell.first;
        const int cellY = cell.second;

        const int minWallX = (std::max)(0, cellX - 1);
        const int maxWallX =
            (std::min)(BOARD_SIZE - 2, cellX + 1);

        const int minWallY = (std::max)(0, cellY - 1);
        const int maxWallY =
            (std::min)(BOARD_SIZE - 2, cellY + 1);

        for (int wallY = minWallY;
            wallY <= maxWallY;
            ++wallY)
        {
            for (int wallX = minWallX;
                wallX <= maxWallX;
                ++wallX)
            {
                for (int direction = 0;
                    direction < 2;
                    ++direction)
                {
                    if (seen[wallX][wallY][direction])
                    {
                        continue;
                    }

                    seen[wallX][wallY][direction] = true;

                    const bool isVertical =
                        direction == 0;

                    if (!CanPlaceWallWithReachability(
                        state,
                        wallX,
                        wallY,
                        isVertical))
                    {
                        continue;
                    }

                    BOARDSTATE nextState = state;

                    // íºëOÇ…çáñ@ê´ÇämîFçœÇ›Ç»ÇÃÇ≈çƒåüç∏ÇµÇ»Ç¢
                    ApplyWallUnchecked(
                        nextState,
                        playerIndex,
                        wallX,
                        wallY,
                        isVertical);

                    const int afterOpponentDistance =
                        GetShortestDistance(
                            nextState,
                            state.playerX[opponentIndex],
                            state.playerY[opponentIndex],
                            opponentGoalY);

                    const int afterPlayerDistance =
                        GetShortestDistance(
                            nextState,
                            state.playerX[playerIndex],
                            state.playerY[playerIndex],
                            goalY);

                    CpuAction action{};
                    action.isWall = true;
                    action.x = wallX;
                    action.y = wallY;
                    action.isVertical = isVertical;

                    // ëäéËÇêLÇŒÇµÅAé©ï™ÇêLÇŒÇ≥Ç»Ç¢ï«ÇóDêÊ
                    const int score =
                        (afterOpponentDistance -
                            opponentDistance) * 100 -
                        (afterPlayerDistance -
                            playerDistance) * 25;

                    wallActions.push_back({
                        action,
                        score
                        });
                }
            }
        }
    }

    std::sort(
        wallActions.begin(),
        wallActions.end(),
        [](const ScoredWall& left,
            const ScoredWall& right)
        {
            return left.score > right.score;
        });

    const int wallCount =
        (std::min)(
            maxWallActions,
            static_cast<int>(
                wallActions.size()));

    for (int i = 0; i < wallCount; ++i)
    {
        actions.push_back(
            wallActions[i].action);
    }

    return actions;
}

int QuoridorCpu::Minimax(
    const BOARDSTATE& state,
    int depth,
    int currentPlayerIndex,
    int alpha,
    int beta,
    bool useWall) const
{
    if (depth <= 0 ||
        IsGoal(state, 0) ||
        IsGoal(state, 1))
    {
        return Evaluate(state);
    }

    const auto actions =
        GenerateMoves(
            state,
            currentPlayerIndex,
            useWall,
            SEARCH_MAX_WALL_ACTIONS);

    if (actions.empty())
    {
        return Evaluate(state);
    }

    if (currentPlayerIndex == 1)
    {
        int bestScore =
            (std::numeric_limits<int>::min)();

        for (const CpuAction& action : actions)
        {
            BOARDSTATE nextState = state;

            // GenerateMoves()Ç≈çáñ@ê´ämîFçœÇ›
            ApplyActionUnchecked(
                nextState,
                currentPlayerIndex,
                action);

            const int score =
                Minimax(
                    nextState,
                    depth - 1,
                    0,
                    alpha,
                    beta,
                    useWall);

            bestScore =
                (std::max)(bestScore, score);

            alpha =
                (std::max)(alpha, bestScore);

            if (alpha >= beta)
            {
                break;
            }
        }

        return bestScore;
    }

    int bestScore =
        (std::numeric_limits<int>::max)();

    for (const CpuAction& action : actions)
    {
        BOARDSTATE nextState = state;

        ApplyActionUnchecked(
            nextState,
            currentPlayerIndex,
            action);

        const int score =
            Minimax(
                nextState,
                depth - 1,
                1,
                alpha,
                beta,
                useWall);

        bestScore =
            (std::min)(bestScore, score);

        beta =
            (std::min)(beta, bestScore);

        if (alpha >= beta)
        {
            break;
        }
    }

    return bestScore;
}

bool QuoridorCpu::FindBestAction(
    const QuoridorBoard& board,
    const Player players[2],
    CpuAction& outAction,
    int searchDepth) const
{
    const int totalStart = GetNowCount();

    const BOARDSTATE state =
        CreateState(board, players);

    const int playerDistance =
        GetShortestDistance(
            state,
            state.playerX[0],
            state.playerY[0],
            BOARD_SIZE - 1);

    const int cpuDistance =
        GetShortestDistance(
            state,
            state.playerX[1],
            state.playerY[1],
            0);

    // Pythonî≈Ç∆ìØÇ∂ìÆìIíTçıêßå‰
    int depth = 3;
    bool useWall = true;

    if (cpuDistance <= 4)
    {
        depth = 2;
        useWall = false;
    }
    else if (std::abs(playerDistance - cpuDistance) >= 3)
    {
        depth = 2;
        useWall = true;
    }

    // åƒÇ—èoÇµë§Ç™è¨Ç≥Ç¢ê[ìxÇéwíËÇµÇΩèÍçáÇÕè„å¿Ç∆ÇµÇƒë∏èd
    if (searchDepth > 0)
    {
        depth = (std::min)(depth, searchDepth);
    }

    const int generateStart = GetNowCount();

    const auto actions =
        GenerateMoves(
            state,
            1,
            useWall,
            ROOT_MAX_WALL_ACTIONS);

    const int generateTime =
        GetNowCount() - generateStart;

    if (actions.empty())
    {
        return false;
    }

    int bestScore =
        (std::numeric_limits<int>::min)();

    bool foundAction = false;

    const int minimaxStart = GetNowCount();

    for (const CpuAction& action : actions)
    {
        BOARDSTATE nextState = state;

        ApplyActionUnchecked(
            nextState,
            1,
            action);

        const int score =
            Minimax(
                nextState,
                depth - 1,
                0,
                bestScore,
                (std::numeric_limits<int>::max)(),
                useWall);

        if (!foundAction ||
            score > bestScore)
        {
            bestScore = score;
            outAction = action;
            foundAction = true;
        }
    }

    const int minimaxTime =
        GetNowCount() - minimaxStart;

    const int totalTime =
        GetNowCount() - totalStart;

    //printfDx(
    //    "Depth=%d UseWall=%d Generate=%d Minimax=%d Actions=%d Total=%d\n",
    //    depth,
    //    useWall ? 1 : 0,
    //    generateTime,
    //    minimaxTime,
    //    static_cast<int>(actions.size()),
    //    totalTime);

    return foundAction;
}