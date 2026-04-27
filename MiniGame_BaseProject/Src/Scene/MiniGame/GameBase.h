#pragma once
#include "../../Manager/Camera.h"
class ResourceManager;
class Setting;

class GameBase
{
public:

	// コンストラクタ
	GameBase(void);

	// デストラクタ
	virtual ~GameBase(void) = 0;

	// 初期化処理
	virtual void Init(void) = 0;

	// 更新ステップ
	virtual void Update(void) = 0;

	// 描画処理
	virtual void Draw(void) = 0;
	virtual void DrawUI(void) = 0;

	// ゲームの破棄
	virtual void Reset(void) = 0;

protected:

	// リソース管理
	ResourceManager& resMng_;

	// 設定管理
	// 描画位置とかの設定をここから取る
	Setting& setting_;
};

