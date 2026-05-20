#pragma once
#include "GameBase.h"


class FirstPressGame : public GameBase
{
public:

	enum class GameState
	{
		WAIT,
		READY,
		GO,
		RESULT
	};

	FirstPressGame(void);
	~FirstPressGame(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;
	void Reset(void) override;

private:

	GameState gameState_;

	// 背景
	int backImg_;
	int backUIImg_;
	int readyUIImg_;
	int pressUIImg_;
	int winUIImg_;
	int loseUIImg_;
	int countUIImg_;
	int winCountUIImg_;
	int countUI2Img_;
	int loseCountUIImg_;
	int pointUIImg_;
	int lostUIImg_;
	int playerTimeUIImg_;

	// フレームカウント
	int timer_;

	// ランダム待機時間
	int waitTime_;

	// 押したかどうか
	bool isPressed_;

	// 押したフレーム
	int pressFrame_;

	// CPUの押したフレーム
	int cpuPressFrame_;

	// CPUが押したかどうか
	bool cpuPressed_;

	int round_;
	int playerWin_;
	int cpuWin_;

	int readyAlpha_;
	bool isFlash_;

	void ResetRound(void);
	void DrawFadeReady(void);
	void DrawCountUI(void);
	void Flash(void);
};

