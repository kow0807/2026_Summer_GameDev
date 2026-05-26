#pragma once
#include <memory>
#include <string>
#include "GameBase.h"
#include "../../Object/Actor/Quoridor/QuoridorPlayer.h"
#include "../../Object/Actor/Quoridor/PythonAI.h"

class Desk;
class Board;
class Wall;
class PlayerPiece;
class PythonAI;

class Quoridor : public GameBase
{
public:

	// 定数
	// ----------------------------
	const static int BOARD_SIZE = 9; // ボードのサイズ
	const static int MAX_WALLS = 10; // 各プレイヤーの壁最大数

	enum class GAME_MODE
	{
		NONE,
		CPU,
		PLAYER
	};

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

	// ゲームモード
	GAME_MODE gameMode_;

	// プレイヤーモード
	MODE mode_;

	// プレイヤー
	Player players_[2];

	// プレイヤーのターン
	int currentTurn_;

	// プレイヤーターンフラグ
	bool isChangeTurn_;

	// 壁カーソル
	int wallCursorX_;
	int wallCursorY_;
	bool wallVertical_;

	// ゲームオーバー
	bool isGameOver_;
	int  winner_;

	// 移動先候補（ハイライト用）
	std::vector<std::pair<int, int>> moveCandidates_;

	// 座標変換
	VECTOR GetWorldPos(int x, int y) const;
	VECTOR GetCellCenter(int x, int y) const;

	// 描画関連
	void DrawBoard(void);
	void DrawPlayers(void);
	void DrawWallAndGrid(void);
	void DrawMoveCandidates(void);
	void DrawGameOver(void);

	// ボックス描画ユーティリティ
	void DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag);
	void DrawCube3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag);

	VECTOR MakeMin(VECTOR a, VECTOR b);
	VECTOR MakeMax(VECTOR a, VECTOR b);

	// 候補リスト更新
	void RefreshMoveCandidates(void);
	
	// 点滅判定
	bool IsBlink(void) const;

	// 描画用オブジェクト
	std::unique_ptr<Desk> desk_;
	std::unique_ptr<Board> board_;
	std::unique_ptr<Wall> previewWall_;
	std::unique_ptr<PlayerPiece> playerPieces_[2];
	std::unique_ptr<PythonAI> pythonAI_;

	// CPU思考中フラグ
	bool isCpuThinking_;

	void ApplyCpuMove(const std::string& json);
	std::string BuildBoardJson(void) const;

	// プレイヤー・CPUの処理を分離
	void UpdatePlayer(void);
	void UpdateCpu(void);

	// タイトル用
	int fontTitle_;

	// 通常文字用
	int fontMain_;

};
