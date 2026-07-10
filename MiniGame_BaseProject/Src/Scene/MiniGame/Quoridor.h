#pragma once
#include <memory>
#include <string>
#include <array>
#include "GameBase.h"
#include "../../Object/Actor/Quoridor/QuoridorPlayer.h"
#include "../../Object/Actor/Quoridor/PythonAI.h"

class Desk;
class QuoridorBoard;
class Wall;
class PlayerPiece;
class PythonAI;
class Triangle;

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

	static void SetDebugLogPath(const std::string& path);
	static std::string GetDebugLogPath();

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

	// 移動モード中の選択カーソル管理用
	int moveCursorIndex_;

	// 移動先候補（ハイライト用）
	std::vector<std::pair<int, int>> moveCandidates_;

	// seハンドル
	int pMH_;// 駒が動いたとき
	int wMH_;// 壁が動いたとき
	int wRH_;// 壁が回転した時
	int mCH_;// モードが切り替わった時
	int vicH_;//勝利した時
	int defH_;//敗北した時

	// Pause用	
	int menuSe_;
	int cancelSe_;
	int moveSe_;
	int decideSEH_;
	bool isPause_;
	int pauseScreenHandle_;
	float pauseX_;
	int pauseSelect_;
	int explanationFontHandle_;

	// 斜め移動中のフラグ
	bool isDiagonalSelect_;
	
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
	std::unique_ptr<QuoridorBoard> board_;
	std::unique_ptr<Wall> previewWall_;
	std::unique_ptr<PlayerPiece> playerPieces_[2];
	std::unique_ptr<PythonAI> pythonAI_;
	std::array<std::unique_ptr<Triangle>, 8> moveTriangleIndicators_; // 移動方向表示用の三角形オブジェクト


	// CPU思考中フラグ
	bool isCpuThinking_;

	// 思考時間の演出用タイマー
	int cpuStartTime_;

	void ApplyCpuMove(const std::string& json);
	std::string BuildBoardJson(void) const;

	// プレイヤー・CPUの処理を分離
	void UpdatePlayer(void);
	void UpdateCpu(void);

	// タイトル用
	int fontTitle_;

	// 通常文字用
	int fontMain_;

	// フレームカウンタ（ミニゲーム終了用）
	int rFrameCount_;

	// 壁残りに関する警告
	bool isWallWarningActive_;
	int wallWarningTimer_;

	bool isDiagChoosing_ = false;
	std::pair<int, int> diagTarget_ = { 0, 0 };

	static std::string debugLogPath_;

	// Pause用
	bool PauseUpdate(void);
	void PauseDraw(void);
};
