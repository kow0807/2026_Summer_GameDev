#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class OverScene : public SceneBase
{

public:

	// 定数
	// ----------------------------

	// ----------------------------

	// コンストラクタ
	OverScene(void);

	// デストラクタ
	~OverScene(void);

	// 初期化処理
	void Init(void) override;

	// 毎フレームの更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

	// 描画処理(UI)
	void DrawUI(void) override;

private:
	
};
