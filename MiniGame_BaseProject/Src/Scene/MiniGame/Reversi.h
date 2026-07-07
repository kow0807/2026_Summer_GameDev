#pragma once
#include "GameBase.h"

class Reversi : public GameBase
{
public:

	enum class GameState
	{
		WAIT,
		READY,
		GO,
		RESULT
	};

	enum Stone
	{
		EMPTY,
		BLACK,
		WHITE
	};

	Reversi(void);
	~Reversi(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;
	void Reset(void) override;

private:
	GameState gameState_;

	Stone board_[8][8];

	int yuMinchoFontHandle_;

	int backImg_;

	bool isBgm_;
	int bgm_;
	int pieceSe_;
	bool isResult_;
	int clearSe_;
	int overSe_;
	bool isSkip_;
	int skipSe_;

	int cursorX_;
	int cursorY_;

	bool playerTurn_;

	int blackCount_;
	int whiteCount_;

	// CPUévçléûä‘
	int cpuThinkTimer_;

	// CPUÇÃíÖéËêî
	int cpuMoveCount_;

	bool skipMessage_;
	int skipTimer_;
	bool playerSkipped_;

	int resultTimer_;

	bool CanPlace(int x, int y, Stone stone);
	void FlipStone(int x, int y, Stone stone);
	bool HasValidMove(Stone stone);

	int CountFlip(int x, int y, Stone stone);
	int EvaluateMove(int x, int y);

	void CPUAction();

	void UpdateStoneCount();
};