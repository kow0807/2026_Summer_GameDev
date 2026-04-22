#pragma once
#include "SceneBase.h"
class PixelMaterial;
class PixelRenderer;


class GameScene : public SceneBase
{

public:

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

};
