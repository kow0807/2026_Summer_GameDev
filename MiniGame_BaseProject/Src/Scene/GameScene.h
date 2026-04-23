#pragma once
#include "SceneBase.h"
class PixelMaterial;
class PixelRenderer;


class GameScene : public SceneBase
{

public:

	enum class M_STATE
	{
		FIRST_PRESS,
		QUIZ,
		REVERSI,
		BUTTON_MASH,
		FLASH_CALC,
		QUORIDOR,
		HARE_AND_HOUNDS,
		MINI_SHOGI,
		MAX
	};

	// 定数
	// ----------------------------

	// ----------------------------

	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;

private:

	M_STATE mState_;

	// ミニゲーム選択操作
	void SelectGameUpdate(void);

	// ミニゲーム選択UI
	void SelectGameDrawUI(void);
};
