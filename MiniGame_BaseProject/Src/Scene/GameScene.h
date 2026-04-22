#pragma once
#include "SceneBase.h"
class PixelMaterial;
class PixelRenderer;


class GameScene : public SceneBase
{

public:

	// 定数
	// ----------------------------

	// ミニゲーム数
	static constexpr int M_MAX_SELECT = 8;

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

	// ミニゲーム選択状態
	int mSelectIndex_;

	// ミニゲーム選択操作
	void SelectGameUpdate(void);

	// ミニゲーム選択UI
	void SelectGameDrawUI(void);
};
