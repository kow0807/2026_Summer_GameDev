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
};

