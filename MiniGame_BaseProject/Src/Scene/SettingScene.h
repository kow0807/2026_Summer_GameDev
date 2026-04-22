#pragma once
#include <memory>
#include "SceneBase.h"
#include "../Manager/SettingManager.h"

class SettingScene : public SceneBase
{
public:

	// コンストラクタ
	SettingScene(void);

	// デストラクタ
	~SettingScene(void);
	
	// 初期化処理
	void Init(void) override;
	
	// 毎フレームの更新処理
	void Update(void) override;
	
	// 描画処理
	void Draw(void) override;
	
	// 描画処理(UI)
	void DrawUI(void) override;

private:

	// 設定管理

};

