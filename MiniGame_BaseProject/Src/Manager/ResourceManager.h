#pragma once
#include <map>
#include <string>
#include <memory>
#include "Resource.h"

class ResourceManager
{

public:

	// リソース名
	enum class SRC
	{
		FIRST_PRESS_GAME_BACK,				// 背景
		FIRST_PRESS_GAME_BACK_UI,			// 背景UI
		FIRST_PRESS_GAME_READY_UI,			// 準備UI
		FIRST_PRESS_GAME_PRESS_UI,			// 押せ！UI
		FIRST_PRESS_GAME_WIN_UI,			// 勝利UI
		FIRST_PRESS_GAME_LOSE_UI,			// 敗北UI
		FIRST_PRESS_GAME_COUNT_UI,			// カウントUI
		FIRST_PRESS_GAME_WIN_COUNT_UI,		// 勝利カウントUI
		FIRST_PRESS_GAME_COUNT_UI_2,		// カウントUI2
		FIRST_PRESS_GAME_LOSE_COUNT_UI,		// 敗北カウントUI
		FIRST_PRESS_GAME_POINT_UI,			// ポイントUI
		FIRST_PRESS_GAME_LOST_UI,			// ロストUI
		FIRST_PRESS_GAME_PLAYER_TIME_UI,	// プレイヤー時間UI
		BOARD,								// ボード
	};

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static ResourceManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切替時に一旦解放)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	// リソースのロード
	const Resource& Load(SRC src);

	// リソースの複製ロード(モデル用)
	int LoadModelDuplicate(SRC src);

private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理の対象
	std::map<SRC, std::shared_ptr<Resource>> resourcesMap_;

	// 読み込み済みリソース
	std::map<SRC, Resource&> loadedMap_;

	Resource dummy_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(SRC src);

};
