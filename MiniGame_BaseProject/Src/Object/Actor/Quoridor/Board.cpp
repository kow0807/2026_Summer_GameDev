#include <queue>
#include <cstring>
#include "Board.h"

Board::Board(void)
    :
    verticalWalls_(false),
    horizontalWalls_(false)
{
}

Board::~Board(void)
{
}

void Board::Init()
{
    memset(verticalWalls_, 0, sizeof(verticalWalls_));
    memset(horizontalWalls_, 0, sizeof(horizontalWalls_));
}

bool Board::CanMove(int x, int y, int dx, int dy)
{
    int nx = x + dx;
    int ny = y + dy;

    if (nx < 0 || nx >= BOARD_SIZE ||
        ny < 0 || ny >= BOARD_SIZE)
        return false;

    if (dx == 1 && verticalWalls_[x][y]) return false;
    if (dx == -1 && verticalWalls_[x - 1][y]) return false;
    if (dy == 1 && horizontalWalls_[x][y]) return false;
    if (dy == -1 && horizontalWalls_[x][y - 1]) return false;

    return true;
}

bool Board::IsOccupied(int x, int y, Player players[2])
{
    for (int i = 0; i < 2; i++)
    {
        if (players[i].x_ == x && players[i].y_ == y)
            return true;
    }
    return false;
}

bool Board::CanReachGoal(Player& p, int goalY, Player players[2])
{
    bool visited[BOARD_SIZE][BOARD_SIZE] = {};

    std::queue<std::pair<int, int>> q;
    q.push({ p.x_, p.y_ });
    visited[p.x_][p.y_] = true;

    int dx[4] = { 1,-1,0,0 };
    int dy[4] = { 0,0,1,-1 };

    while (!q.empty())
    {
        auto [x, y] = q.front(); q.pop();

        if (y == goalY) return true;

        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || nx >= BOARD_SIZE ||
                ny < 0 || ny >= BOARD_SIZE)
                continue;

            if (visited[nx][ny]) continue;
            if (!CanMove(x, y, dx[i], dy[i])) continue;

            visited[nx][ny] = true;
            q.push({ nx, ny });
        }
    }

    return false;
}

bool Board::PlaceWall(int x, int y, bool isVertical, Player players[2])
{
    // 仮置き
    if (isVertical)
    {
        if (x < 0 || x >= BOARD_SIZE - 1 || y < 0 || y >= BOARD_SIZE)
            return false;

        if (verticalWalls_[x][y]) return false;
        verticalWalls_[x][y] = true;
    }
    else
    {
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE - 1)
            return false;

        if (horizontalWalls_[x][y]) return false;
        horizontalWalls_[x][y] = true;
    }

    // BFSチェック
    if (!CanReachGoal(players[0], BOARD_SIZE - 1, players) ||
        !CanReachGoal(players[1], 0, players))
    {
        // 戻す
        if (isVertical)
            verticalWalls_[x][y] = false;
        else
            horizontalWalls_[x][y] = false;

        return false;
    }

    return true;
}