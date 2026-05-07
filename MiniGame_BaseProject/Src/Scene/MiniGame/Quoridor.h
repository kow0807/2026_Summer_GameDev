#pragma once
#include <memory>
#include "GameBase.h"
#include "../../Object/Actor/Quoridor/QuoridorPlayer.h"

class Desk;
class Board;
class Wall;

class Quoridor : public GameBase
{
public:

	// 定数
	// ----------------------------
	const static int BOARD_SIZE = 9; // ボードのサイズ
	const static int MAX_WALLS = 20; // 壁の最大数
	constexpr static float CELL_SIZE = 50.0f; // セルのサイズ

	enum class MODE
	{
		NONE,
		MOVE,
		WALL
	};

	Quoridor(void);
	~Quoridor(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;
	void Reset(void) override;

private:

	// プレイヤーモード
	MODE mode_;

	// プレイヤー
	Player players_[2];

	// 壁の配置を管理するための配列
	// 縦壁を管理するための配列
	bool verticalWalls_[BOARD_SIZE - 1][BOARD_SIZE];

	// 横壁を管理するための配列
	bool horizontalWalls_[BOARD_SIZE][BOARD_SIZE - 1];

	// 上、右、下、左の順で壁の有無を管理
	bool walls_[BOARD_SIZE][BOARD_SIZE][4];

	// プレイヤーのターン
	int currentTurn_;

	// プレイヤーターンフラグ
	bool isChageTurn_;

	// 壁カーソル
	int wallCursorX_;
	int wallCursorY_;

	bool wallVertical_;

	// 座標変換
	VECTOR GetWorldPos(int x, int y);

	// 壁用
	VECTOR GetCellCenter(int x, int y);

	// 描画関連
	void DrawBoard(void);
	void DrawPlayers(void);
	void DrawWall(void);
	void DrawWallCursor(void);

	VECTOR MakeMin(VECTOR a, VECTOR b);
	VECTOR MakeMax(VECTOR a, VECTOR b);

	// デバック用
	void DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag);
	void DrawCube3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag);

	// デスク(テーブル代わり)
	std::unique_ptr<Desk> desk_;

	// ボード
	std::unique_ptr<Board> board_;

	// プレビュー用
	std::unique_ptr<Wall> previewWall_;

	// 壁サイズ
	float wallLength_ = CELL_SIZE * 2.0f;
	float thickness_ = 5.0f;
	float height_ = 20.0f;

};

