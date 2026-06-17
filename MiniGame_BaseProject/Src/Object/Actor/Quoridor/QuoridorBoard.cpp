#include <queue>
#include <cstring>
#include <DxLib.h>
#include "BoardBase.h"
#include "Grid.h"
#include "Wall.h"
#include "QuoridorBoard.h"

QuoridorBoard::QuoridorBoard(void)
    :
    verticalWalls_(false),
    horizontalWalls_(false)
{
}

QuoridorBoard::~QuoridorBoard(void)
{
}

void QuoridorBoard::Init()
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

void QuoridorBoard::Draw(void)
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
bool QuoridorBoard::CanMove(int x, int y, int dx, int dy) const
{
    int nx = x + dx;
    int ny = y + dy;

    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
        return false;

    // 1. 右への移動 (dx == 1) : マス(x,y) と (x+1,y) の間の縦壁
    if (dx == 1) {
        if (x >= 0 && x < BOARD_SIZE - 1) {
            if (y < BOARD_SIZE - 1 && verticalWalls_[x][y]) return false;
            if (y > 0 && verticalWalls_[x][y - 1]) return false;
        }
    }
    // 2. 左への移動 (dx == -1) : マス(x-1,y) と (x,y) の間の縦壁
    else if (dx == -1) {
        int cx = x - 1;
        if (cx >= 0 && cx < BOARD_SIZE - 1) {
            if (y < BOARD_SIZE - 1 && verticalWalls_[cx][y]) return false;
            if (y > 0 && verticalWalls_[cx][y - 1]) return false;
        }
    }
    // 3. 下への移動 (dy == 1) : マス(x,y) と (x,y+1) の間の横壁
    else if (dy == 1) {
        if (y >= 0 && y < BOARD_SIZE - 1) {
            if (x < BOARD_SIZE - 1 && horizontalWalls_[x][y]) return false;
            if (x > 0 && horizontalWalls_[x - 1][y]) return false;
        }
    }
    // 4. 上への移動 (dy == -1) : マス(x,y-1) と (x,y) の間の横壁
    else if (dy == -1) {
        int cy = y - 1;
        if (cy >= 0 && cy < BOARD_SIZE - 1) {
            if (x < BOARD_SIZE - 1 && horizontalWalls_[x][cy]) return false;
            if (x > 0 && horizontalWalls_[x - 1][cy]) return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// 占有チェック
// ---------------------------------------------------------------------------
bool QuoridorBoard::IsOccupied(int x, int y, const Player players[2]) const
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
bool QuoridorBoard::CanReachGoal(const Player& p, int goalY, const Player players[2]) const
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
std::vector<std::pair<int, int>> QuoridorBoard::GetMoveCandidates(
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
std::vector<std::pair<int, int>> QuoridorBoard::GetAllMoveCandidates(
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
int QuoridorBoard::CheckWinner(const Player players[2]) const
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
bool QuoridorBoard::CanPlaceWall(int x, int y, bool isVertical) const
{
    // 盤面外（交差点は 0 ～ BOARD_SIZE-2 まで）
    if (x < 0 || x >= BOARD_SIZE - 1 ||
        y < 0 || y >= BOARD_SIZE - 1)
        return false;

    if (isVertical)
    {
        // 1. 縦壁同士の重複・半分重なりをチェック（2マス分）
        if (verticalWalls_[x][y]) return false;
        if (y > 0 && verticalWalls_[x][y - 1]) return false;
        if (y < BOARD_SIZE - 2 && verticalWalls_[x][y + 1]) return false;

        // 2. 横壁との交差チェック（コリドールのルール：中心の交差点が被る場合のみ禁止）
        // 横壁と横壁の「隙間」に縦壁を入れる場合、中心点が被らなければ設置可能です
        if (horizontalWalls_[x][y]) return false;
    }
    else
    {
        // 1. 横壁同士の重複・半分重なりをチェック（2マス分）
        if (horizontalWalls_[x][y]) return false;
        if (x > 0 && horizontalWalls_[x - 1][y]) return false;
        if (x < BOARD_SIZE - 2 && horizontalWalls_[x + 1][y]) return false;

        // 2. 縦壁との交差チェック（中心の交差点が被る場合のみ禁止）
        if (verticalWalls_[x][y]) return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// 壁設置（BFSチェック付き）
// 失敗時は状態を変えずに false を返す
// ---------------------------------------------------------------------------
bool QuoridorBoard::PlaceWall(int x, int y, bool isVertical, Player players[2], VECTOR wallColor)
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
        verticalWalls_[x][y] = true; // ★1マスだけ true にする
    }
    else
    {
        horizontalWalls_[x][y] = true; // ★1マスだけ true にする
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
        }
        else
        {
            horizontalWalls_[x][y] = false;
        }
        return false;
    }

    // --- Wallオブジェクト生成 ---
    auto wall = std::make_unique<Wall>();
	wall->SetCellSize(CELL_SIZE);
    wall->Init();
    wall->SetType(isVertical ? Wall::TYPE::VERTICAL : Wall::TYPE::HORIZONTAL);
    wall->SetBoardPosition(x, y);
    wall->SetColor(wallColor.x, wallColor.y, wallColor.z);
    wall->RefreshTransform();
    walls_.push_back(std::move(wall));

    return true;
}

// ---------------------------------------------------------------------------
// 壁描画
// ---------------------------------------------------------------------------
void QuoridorBoard::DrawWalls(void)
{
    SetUseBackCulling(FALSE);

    for (auto& wall : walls_)
    {
        wall->Draw();
    }

    SetUseBackCulling(TRUE);
}

bool QuoridorBoard::GetVerticalWall(int x, int y) const
{
    return verticalWalls_[x][y];
}

bool QuoridorBoard::GetHorizontalWall(int x, int y) const
{
    return horizontalWalls_[x][y];
}