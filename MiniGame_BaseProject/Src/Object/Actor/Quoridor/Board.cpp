#include <queue>
#include <cstring>
#include <DxLib.h>
#include "Wall.h"
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

void Board::DrawWalls(void)
{
    SetUseBackCulling(FALSE);

 //   constexpr float WALL_THICKNESS = 10.0f;
 //   constexpr float WALL_HEIGHT = 40.0f;
	//constexpr float WALL_MARGIN = 15.0f;

 //   for (int y = 0; y < BOARD_SIZE; y++)
 //   {
 //       for (int x = 0; x < BOARD_SIZE; x++)
 //       {
 //           //----------------------------------------
 //           // ècï«

 //           if (x < BOARD_SIZE - 1 &&
 //               y < BOARD_SIZE - 1 &&
 //               verticalWalls_[x][y])
 //           {
 //               // èdï°ï`âÊñhé~
 //               if (y > 0 &&
 //                   verticalWalls_[x][y - 1])
 //               {
 //                   continue;
 //               }

 //               VECTOR pos = VAdd(
 //                   VGet(
 //                       x * CELL_SIZE,
 //                       0,
 //                       y * CELL_SIZE
 //                   ),
 //                   VGet(
 //                       CELL_SIZE / 2,
 //                       0,
 //                       CELL_SIZE / 2
 //                   )
 //               );

 //               VECTOR min = VAdd(
 //                   pos,
 //                   VGet(
 //                       -WALL_THICKNESS,
 //                       0,
 //                       -CELL_SIZE + WALL_MARGIN
 //                   )
 //               );

 //               VECTOR max = VAdd(
 //                   pos,
 //                   VGet(
 //                       WALL_THICKNESS,
 //                       WALL_HEIGHT,
 //                       CELL_SIZE - WALL_MARGIN
 //                   )
 //               );

 //               DrawCube3D(
 //                   min,
 //                   max,
 //                   GetColor(30, 30, 30),
 //                   TRUE
 //               );
 //           }

 //           //----------------------------------------
 //           // â°ï«

 //           if (x < BOARD_SIZE - 1 &&
 //               y < BOARD_SIZE - 1 &&
 //               horizontalWalls_[x][y])
 //           {
 //               // èdï°ï`âÊñhé~
 //               if (x > 0 &&
 //                   horizontalWalls_[x - 1][y])
 //               {
 //                   continue;
 //               }

 //               VECTOR pos = VAdd(
 //                   VGet(
 //                       x * CELL_SIZE,
 //                       0,
 //                       y * CELL_SIZE
 //                   ),
 //                   VGet(
 //                       CELL_SIZE / 2,
 //                       0,
 //                       CELL_SIZE / 2
 //                   )
 //               );

 //               VECTOR min = VAdd(
 //                   pos,
 //                   VGet(
 //                       -CELL_SIZE + WALL_MARGIN,
 //                       0,
 //                       -WALL_THICKNESS
 //                   )
 //               );

 //               VECTOR max = VAdd(
 //                   pos,
 //                   VGet(
 //                       CELL_SIZE - WALL_MARGIN,
 //                       WALL_HEIGHT,
 //                       WALL_THICKNESS
 //                   )
 //               );

 //               DrawCube3D(
 //                   min,
 //                   max,
 //                   GetColor(30, 30, 30),
 //                   TRUE
 //               );
 //           }
 //       }
 //   }

    for (auto& wall : walls_)
    {
        wall->Draw();
    }

    SetUseBackCulling(TRUE);
}

bool Board::CanPlaceWall(int x, int y, bool isVertical)
{
    //----------------------------------------
    // ècï«
    if (isVertical)
    {
        // îÕàÕ
        if (x < 0 ||
            x >= BOARD_SIZE - 1 ||
            y < 0 ||
            y >= BOARD_SIZE - 1)
        {
            return false;
        }

        //----------------------------------------
        // èdï°ã÷é~

        if (verticalWalls_[x][y] ||
            verticalWalls_[x][y + 1])
        {
            return false;
        }

        //----------------------------------------
        // åç∑ã÷é~

        if (horizontalWalls_[x][y] &&
            horizontalWalls_[x + 1][y])
        {
            return false;
        }
    }
    //----------------------------------------
    // â°ï«
    else
    {
        if (x < 0 ||
            x >= BOARD_SIZE - 1 ||
            y < 0 ||
            y >= BOARD_SIZE - 1)
        {
            return false;
        }

        //----------------------------------------
        // èdï°ã÷é~

        if (horizontalWalls_[x][y] ||
            horizontalWalls_[x + 1][y])
        {
            return false;
        }

        //----------------------------------------
        // åç∑ã÷é~

        if (verticalWalls_[x][y] &&
            verticalWalls_[x][y + 1])
        {
            return false;
        }
    }

    return true;
}

bool Board::PlaceWall(int x, int y, bool isVertical, Player players[2])
{
    if (isVertical)
    {
        verticalWalls_[x][y] = true;
        verticalWalls_[x][y + 1] = true;

        auto wall = std::make_unique<Wall>();
        wall->Init();
        walls_.push_back(std::move(wall));
    }
    else
    {
        horizontalWalls_[x][y] = true;
        horizontalWalls_[x + 1][y] = true;

        auto wall = std::make_unique<Wall>();
        wall->Init();
        walls_.push_back(std::move(wall));
    }

    // BFSÉ`ÉFÉbÉN
    if (!CanReachGoal(players[0], BOARD_SIZE - 1, players) ||
        !CanReachGoal(players[1], 0, players))
    {
        // ñﬂÇ∑
        if (isVertical)
        {
            verticalWalls_[x][y] = false;
            verticalWalls_[x][y + 1] = false;
        }
        else
        {
            horizontalWalls_[x][y] = false;
            horizontalWalls_[x + 1][y] = false;
        }

        return false;
    }

    return true;
}


void Board::DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{

    // 8í∏ì_
    VECTOR vertexs[8] =
    {
        {min.x,min.y,min.z},
        {max.x,min.y,min.z},
        {max.x,max.y,min.z},
        {min.x,max.y,min.z},

        {min.x,min.y,max.z},
        {max.x,min.y,max.z},
        {max.x,max.y,max.z},
        {min.x,max.y,max.z},
    };

    // ñ Ç≤Ç∆Ç…ï`âÊ
    auto drawFace = [&](int a, int b, int c, int d)
        {
            DrawTriangle3D(vertexs[a], vertexs[b], vertexs[c], color, fillFlag);
            DrawTriangle3D(vertexs[a], vertexs[c], vertexs[d], color, fillFlag);
        };

    // ëOñ 
    drawFace(0, 1, 2, 3);

    // îwñ 
    drawFace(4, 5, 6, 7);

    // ç∂
    drawFace(0, 3, 7, 4);

    // âE
    drawFace(1, 2, 6, 5);

    // è„
    drawFace(3, 2, 6, 7);

    // â∫
    drawFace(0, 1, 5, 4);
}

void Board::DrawCube3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{

    //----------------------------------------
    // í∏ì_

    VECTOR v[8] =
    {
        // ëOñ 
        VGet(min.x, min.y, min.z), // 0
        VGet(max.x, min.y, min.z), // 1
        VGet(max.x, max.y, min.z), // 2
        VGet(min.x, max.y, min.z), // 3

        // îwñ 
        VGet(min.x, min.y, max.z), // 4
        VGet(max.x, min.y, max.z), // 5
        VGet(max.x, max.y, max.z), // 6
        VGet(min.x, max.y, max.z), // 7
    };

    //----------------------------------------
    // éläpå`ï`âÊä÷êî

    auto DrawQuad =
        [&](
            int a,
            int b,
            int c,
            int d
            )
        {
            DrawTriangle3D(
                v[a],
                v[b],
                v[c],
                color,
                fillFlag
            );

            DrawTriangle3D(
                v[a],
                v[c],
                v[d],
                color,
                fillFlag
            );
        };

    //----------------------------------------
    // ëOñ 

    DrawQuad(0, 1, 2, 3);

    //----------------------------------------
    // îwñ 
    // èáî‘ãtì]

    DrawQuad(5, 4, 7, 6);

    //----------------------------------------
    // ç∂ñ 

    DrawQuad(4, 0, 3, 7);

    //----------------------------------------
    // âEñ 

    DrawQuad(1, 5, 6, 2);

    //----------------------------------------
    // è„ñ 

    DrawQuad(3, 2, 6, 7);

    //----------------------------------------
    // íÍñ 

    DrawQuad(4, 5, 1, 0);

}
