#pragma once
#include <memory>
#include "GameBase.h"

class Board;

class Quoridor : public GameBase
{
public:

	// 定数
	// ----------------------------
	const static int BOARD_SIZE = 9; // ボードのサイズ
	const static int MAX_WALLS = 20; // 壁の最大数
	constexpr static float CELL_SIZE = 50.0f; // セルのサイズ

	Quoridor(void);
	~Quoridor(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;
	void Reset(void) override;

private:

	struct Player
	{
		// プレイヤーの位置
		int x_, y_;

		unsigned int color_;
	};
	
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

	// 座標変換
	VECTOR GetWorldPos(int x, int y);


	// 描画関連
	void DrawBoard(void);
	void DrawPlayers(void);

	// デバック用
	void DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag);

	std::unique_ptr<Board> board_;
};

