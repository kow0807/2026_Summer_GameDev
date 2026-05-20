#include <queue>
#include <cstring>
#include <DxLib.h>
#include "BoardBase.h"
#include "Grid.h"
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
    
	boardBase_ = std::make_unique<BoardBase>();
	boardBase_->Init();

    for (int y = 0; y < BOARD_SIZE; y++)
    {
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            auto grid = std::make_unique<Grid>();

            grid->Init();
			grid->SetGridSize(CELL_SIZE);
            grid->SetBoardPosition(x, y);

            grids_.push_back(std::move(grid));
        }
    }

    walls_.clear();
}

void Board::Draw(void)
{
    boardBase_->Draw();

    for (auto& grid : grids_)
    {
        grid->Draw();
    }

    DrawWalls();
}

// ---------------------------------------------------------------------------
// 移動可否
// ---------------------------------------------------------------------------
bool Board::CanMove(int x, int y, int dx, int dy) const
{
    int nx = x + dx;
    int ny = y + dy;

    if (nx < 0 || nx >= BOARD_SIZE ||
        ny < 0 || ny >= BOARD_SIZE)
        return false;

    // dx=+1 : x と x+1 の間の縦壁
    if (dx == 1 && verticalWalls_[x][y]) return false;
    if (dx == -1 && verticalWalls_[x - 1][y]) return false;

    // dy=+1 : y と y+1 の間の横壁
    if (dy == 1 && horizontalWalls_[x][y]) return false;
    if (dy == -1 && horizontalWalls_[x][y - 1]) return false;

    return true;
}

// ---------------------------------------------------------------------------
// 占有チェック
// ---------------------------------------------------------------------------
bool Board::IsOccupied(int x, int y, const Player players[2]) const
{
    for (int i = 0; i < 2; i++)
    {
        if (players[i].x_ == x && players[i].y_ == y)
            return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// BFS ゴール到達確認
// ---------------------------------------------------------------------------
bool Board::CanReachGoal(const Player& p, int goalY, const Player players[2]) const
{
    bool visited[BOARD_SIZE][BOARD_SIZE] = {};

    std::queue<std::pair<int, int>> q;
    q.push({ p.x_, p.y_ });
    visited[p.x_][p.y_] = true;

    const int dx[4] = { 1,-1, 0, 0 };
    const int dy[4] = { 0, 0, 1,-1 };

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

// ---------------------------------------------------------------------------
// 移動先候補（1方向分）
// ---------------------------------------------------------------------------
std::vector<std::pair<int, int>> Board::GetMoveCandidates(
    int x, int y,
    int dx, int dy,
    const Player players[2]) const
{
    std::vector<std::pair<int, int>> result;

    // 壁・盤外チェック
    if (!CanMove(x, y, dx, dy)) return result;

    int nx = x + dx;
    int ny = y + dy;

    // 隣に相手がいるか判定
    bool enemyThere = false;
    for (int i = 0; i < 2; i++)
    {
        if (players[i].x_ == nx && players[i].y_ == ny)
        {
            enemyThere = true;
            break;
        }
    }

    if (!enemyThere)
    {
        // 通常移動
        result.push_back({ nx, ny });
        return result;
    }

    // 相手がいる場合:
    // 直進できるか？
    if (CanMove(nx, ny, dx, dy))
    {
        // 2マス跳び
        result.push_back({ nx + dx, ny + dy });
    }
    else
    {
        // 直進不可（壁 or 盤外）→ 左右斜めジャンプ
        // 左右 = 直進方向を90度回転した2方向
        const int sideDir[2][2] = { {-dy, dx}, {dy, -dx} };

        for (int s = 0; s < 2; s++)
        {
            int sx = sideDir[s][0];
            int sy = sideDir[s][1];

            if (CanMove(nx, ny, sx, sy))
            {
                result.push_back({ nx + sx, ny + sy });
            }
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// 全方向の移動先候補
// ---------------------------------------------------------------------------
std::vector<std::pair<int, int>> Board::GetAllMoveCandidates(
    const Player& player,
    const Player players[2]) const
{
    std::vector<std::pair<int, int>> result;

    const int dx[4] = { 1,-1, 0, 0 };
    const int dy[4] = { 0, 0, 1,-1 };

    for (int i = 0; i < 4; i++)
    {
        auto cands = GetMoveCandidates(
            player.x_, player.y_,
            dx[i], dy[i],
            players
        );
        for (auto& c : cands)
            result.push_back(c);
    }

    return result;
}

// ---------------------------------------------------------------------------
// 勝利判定
// ---------------------------------------------------------------------------
int Board::CheckWinner(const Player players[2]) const
{
    // プレイヤー0: 上(y=0)からスタート → y==BOARD_SIZE-1 で勝利
    if (players[0].y_ == BOARD_SIZE - 1) return 0;
    // プレイヤー1: 下(y=8)からスタート → y==0 で勝利
    if (players[1].y_ == 0)              return 1;
    return -1;
}

// ---------------------------------------------------------------------------
// 壁設置可否プレビュー
// ---------------------------------------------------------------------------
bool Board::CanPlaceWall(int x, int y, bool isVertical) const
{
    if (isVertical)
    {
        // 縦壁: x: 0..7, y: 0..7
        if (x < 0 || x >= BOARD_SIZE - 1 ||
            y < 0 || y >= BOARD_SIZE - 1)
            return false;

        // 重複禁止（2マス分）
        if (verticalWalls_[x][y] ||
            verticalWalls_[x][y + 1])
            return false;

        // 交差禁止
        if (horizontalWalls_[x][y] &&
            horizontalWalls_[x + 1][y])
            return false;
    }
    else
    {
        // 横壁: x: 0..7, y: 0..7
        if (x < 0 || x >= BOARD_SIZE - 1 ||
            y < 0 || y >= BOARD_SIZE - 1)
            return false;

        // 重複禁止
        if (horizontalWalls_[x][y] ||
            horizontalWalls_[x + 1][y])
            return false;

        // 交差禁止
        if (verticalWalls_[x][y] &&
            verticalWalls_[x][y + 1])
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// 壁設置（BFSチェック付き）
// 失敗時は状態を変えずに false を返す
// ---------------------------------------------------------------------------
bool Board::PlaceWall(int x, int y, bool isVertical, Player players[2])
{
    // --- 設置プレビュー判定 ---
    if (!CanPlaceWall(x, y, isVertical)) return false;

    // --- プレイヤーの壁残数チェック ---
    int turn = -1;
    // PlaceWall は Quoridor.cpp 側で currentTurn_ を渡す設計にする
    // ここでは players[0]/players[1] のどちらが手番かを外から管理しているので
    // 残数チェックは呼び出し側で行う（Quoridor.cpp 参照）

    // --- 仮設置 ---
    if (isVertical)
    {
        verticalWalls_[x][y] = true;
        verticalWalls_[x][y + 1] = true;
    }
    else
    {
        horizontalWalls_[x][y] = true;
        horizontalWalls_[x + 1][y] = true;
    }

    // --- BFSで両プレイヤーの通路確認 ---
    bool reachable =
        CanReachGoal(players[0], BOARD_SIZE - 1, players) &&
        CanReachGoal(players[1], 0, players);

    if (!reachable)
    {
        // 設置を取り消す
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

    // --- Wallオブジェクト生成 ---
    auto wall = std::make_unique<Wall>();
	wall->SetCellSize(CELL_SIZE);
    wall->Init();
    wall->SetType(isVertical ? Wall::TYPE::VERTICAL : Wall::TYPE::HORIZONTAL);
    wall->SetBoardPosition(x, y);
    wall->RefreshTransform();
    walls_.push_back(std::move(wall));

    return true;
}

// ---------------------------------------------------------------------------
// 壁描画
// ---------------------------------------------------------------------------
void Board::DrawWalls(void)
{
    SetUseBackCulling(FALSE);

    for (auto& wall : walls_)
    {
        wall->Draw();
    }

    SetUseBackCulling(TRUE);
}

bool Board::GetVerticalWall(int x, int y) const
{
    return verticalWalls_[x][y];
}

bool Board::GetHorizontalWall(int x, int y) const
{
    return horizontalWalls_[x][y];
}

// ---------------------------------------------------------------------------
// デバッグ用描画ユーティリティ（そのまま維持）
// ---------------------------------------------------------------------------
void Board::DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{
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

    auto drawFace = [&](int a, int b, int c, int d)
        {
            DrawTriangle3D(vertexs[a], vertexs[b], vertexs[c], color, fillFlag);
            DrawTriangle3D(vertexs[a], vertexs[c], vertexs[d], color, fillFlag);
        };

    drawFace(0, 1, 2, 3);
    drawFace(4, 5, 6, 7);
    drawFace(0, 3, 7, 4);
    drawFace(1, 2, 6, 5);
    drawFace(3, 2, 6, 7);
    drawFace(0, 1, 5, 4);
}

void Board::DrawCube3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{
    VECTOR v[8] =
    {
        VGet(min.x, min.y, min.z),
        VGet(max.x, min.y, min.z),
        VGet(max.x, max.y, min.z),
        VGet(min.x, max.y, min.z),
        VGet(min.x, min.y, max.z),
        VGet(max.x, min.y, max.z),
        VGet(max.x, max.y, max.z),
        VGet(min.x, max.y, max.z),
    };

    auto DrawQuad = [&](int a, int b, int c, int d)
        {
            DrawTriangle3D(v[a], v[b], v[c], color, fillFlag);
            DrawTriangle3D(v[a], v[c], v[d], color, fillFlag);
        };

    DrawQuad(0, 1, 2, 3);
    DrawQuad(5, 4, 7, 6);
    DrawQuad(4, 0, 3, 7);
    DrawQuad(1, 5, 6, 2);
    DrawQuad(3, 2, 6, 7);
    DrawQuad(4, 5, 1, 0);
}

void Board::DrawDebugCollision(void)
{
    constexpr float CELL = 50.0f;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

    for (int y = 0; y < BOARD_SIZE; y++)
    {
        for (int x = 0; x < BOARD_SIZE - 1; x++)
        {
            if (!verticalWalls_[x][y]) continue;

            VECTOR center = VGet(
                x * CELL + CELL * 0.5f,
                5.0f,
                y * CELL
            );

            VECTOR min = VAdd(center, VGet(-3.0f, 0.0f, 0.0f));
            VECTOR max = VAdd(center, VGet(3.0f, 30.0f, CELL));

            DrawCube3D(min, max, GetColor(255, 0, 0), TRUE);
        }
    }

    for (int y = 0; y < BOARD_SIZE - 1; y++)
    {
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            if (!horizontalWalls_[x][y]) continue;

            VECTOR center = VGet(
                x * CELL,
                5.0f,
                y * CELL + CELL * 0.5f
            );

            VECTOR min = VAdd(center, VGet(0.0f, 0.0f, -3.0f));
            VECTOR max = VAdd(center, VGet(CELL, 30.0f, 3.0f));

            DrawCube3D(min, max, GetColor(0, 0, 255), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
