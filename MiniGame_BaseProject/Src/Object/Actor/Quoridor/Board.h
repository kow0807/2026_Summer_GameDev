#pragma once
#include "QuoridorPlayer.h"

class Board
{
public:
    // 定数
    // ----------------------------
    const static int BOARD_SIZE = 9; // ボードのサイズ
    const static int MAX_WALLS = 20; // 壁の最大数
    constexpr static float CELL_SIZE = 50.0f; // セルのサイズ

    Board(void);
    ~Board(void);

    void Init(void);

    bool CanMove(int x, int y, int dx, int dy);
    bool IsOccupied(int x, int y, Player players[2]);

    bool PlaceWall(int x, int y, bool isVertical, Player players[2]);
    bool CanReachGoal(Player& p, int goalY, Player players[2]);

private:

    bool verticalWalls_[BOARD_SIZE - 1][BOARD_SIZE];
    bool horizontalWalls_[BOARD_SIZE][BOARD_SIZE - 1];
};

