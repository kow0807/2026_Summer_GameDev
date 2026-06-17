#pragma once
#include <vector>
#include <memory>
#include "QuoridorPlayer.h"


class Grid;
class BoardBase;
class Wall;

class QuoridorBoard
{
public:
    // 定数
    // ----------------------------
    const static int BOARD_SIZE = 9; // ボードのサイズ
    const static int MAX_WALLS = 10; // 各プレイヤーの壁最大数
    constexpr static float CELL_SIZE = 25.0f; // セルのサイズ

    QuoridorBoard(void);
    ~QuoridorBoard(void);

    void Init(void);
    void Draw(void);

    // 移動可否（壁チェック・範囲チェック）
    bool CanMove(int x, int y, int dx, int dy) const;

    // マスが占有されているか
    bool IsOccupied(int x, int y, const Player players[2]) const;

    // 壁設置（BFS通過確認つき）
    // 成功→true、失敗（詰まる・重複・枚数切れ）→false
    bool PlaceWall(int x, int y, bool isVertical, Player players[2], VECTOR wallColor);

    // 壁設置プレビュー判定（実際には設置しない）
    bool CanPlaceWall(int x, int y, bool isVertical) const;

    // BFSでゴール到達可能か
    bool CanReachGoal(const Player& p, int goalY, const Player players[2]) const;

    // 移動先候補リスト（斜めジャンプ含む）
    // 戻り値: 移動可能な (nx, ny) ペアの配列
    std::vector<std::pair<int, int>> GetMoveCandidates(
        int x, int y,
        int dx, int dy,
        const Player players[2]) const;

    // 全方向の移動先候補
    std::vector<std::pair<int, int>> GetAllMoveCandidates(
        const Player& player,
        const Player players[2]) const;

    // 勝利判定
    // プレイヤー0: y==BOARD_SIZE-1 で勝利
    // プレイヤー1: y==0 で勝利
    // 勝者番号(0 or 1)を返す。勝者なし→-1
    int CheckWinner(const Player players[2]) const;

    void DrawWalls(void);

    bool GetVerticalWall(int x, int y) const;
	bool GetHorizontalWall(int x, int y) const;

private:

    // verticalWalls_[x][y]: x と x+1 の間、行 y に縦壁があるか
    // x: 0..BOARD_SIZE-2, y: 0..BOARD_SIZE-1
    bool verticalWalls_[BOARD_SIZE - 1][BOARD_SIZE];

    // horizontalWalls_[x][y]: y と y+1 の間、列 x に横壁があるか
    // x: 0..BOARD_SIZE-1, y: 0..BOARD_SIZE-2
    bool horizontalWalls_[BOARD_SIZE][BOARD_SIZE - 1];


	std::vector<std::unique_ptr<Grid>> grids_;
    std::unique_ptr<BoardBase> boardBase_;
    std::vector<std::unique_ptr<Wall>> walls_;
};
