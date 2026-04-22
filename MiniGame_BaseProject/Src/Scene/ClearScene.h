#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class ClearScene : public SceneBase
{
public:

	// 定数
	// ----------------------------

	// ----------------------------

	// コンストラクタ
	ClearScene(void);

	// デストラクタ
	~ClearScene(void);

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
