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
		FIRST_PRESS_GAME_FLYING_UI,			// フライングUI
		FIRST_PRESS_GAME_EXPLANATION,		// 説明画像
		FIRST_PRESS_GAME_BGM,				// BGM
		FIRST_PRESS_GAME_PRESS_SE,			// Press音
		FIRST_PRESS_GAME_POINT_SE,			// Point音
		FIRST_PRESS_GAME_LOST_SE,			// Lost音
		FIRST_PRESS_GAME_CLEAR_SE,			// クリア音
		FIRST_PRESS_GAME_OVER_SE,			// 敗北音
		FIRST_PRESS_GAME_FALLING_SE,		// 落下音
		FIRST_PRESS_GAME_NOISE_SE,			// ノイズ音
		FIRST_PRESS_GAME_COUNT_SE,			// カウント音
		FIRST_PRESS_GAME_ERROR_SE,			// エラー音
		FIRST_PRESS_GAME_TRIVIA_SE,			// トリビア音

		BUTTON_MASH_GAME_BACK,				// 背景
		BUTTON_MASH_GAME_BACK_UI,			// 背景UI
		BUTTON_MASH_GAME_READY_UI,			// 準備UI
		BUTTON_MASH_GAME_PRESS_UI,			// 押せ！UI
		BUTTON_MASH_GAME_WIN_UI,			// 勝利UI
		BUTTON_MASH_GAME_LOSE_UI,			// 敗北UI
		BUTTON_MASH_GAME_COUNT_UI,			// カウントUI
		BUTTON_MASH_GAME_WIN_COUNT_UI,		// 勝利カウントUI
		BUTTON_MASH_GAME_COUNT_UI_2,		// カウントUI2
		BUTTON_MASH_GAME_LOSE_COUNT_UI,		// 敗北カウントUI
		BUTTON_MASH_GAME_POINT_UI,			// ポイントUI
		BUTTON_MASH_GAME_LOST_UI,			// ロストUI
		BUTTON_MASH_GAME_EXPLANATION,		// 説明画像
		BUTTON_MASH_GAME_READY_SE,			// Ready音
		BUTTON_MASH_GAME_BGM,				// BGM
		BUTTON_MASH_GAME_PUSH_SE,			// Push音
		BUTTON_MASH_GAME_CLEAR_SE,			// クリア音
		BUTTON_MASH_GAME_OVER_SE,			// ゲームオーバー音
		BUTTON_MASH_GAME_POINT_SE,			// ポイント音
		BUTTON_MASH_GAME_LOST_SE,			// ロスト音


		QUIZ_GAME_BACK,						// 背景
		QUIZ_GAME_EXPLANATION,				// 説明画像
		QUIZ_GAME_READY_SE,					// Ready音
		QUIZ_GAME_BGM,						// BGM
		QUIZ_GAME_CORRECT,					// 正解
		QUIZ_GAME_BUZZER,					// 不正解
		QUIZ_GAME_CURSOR,					// カーソル
		QUIZ_GAME_RESULT,					// 結果


		REVERSI_BACK,						// 背景
		REVERSI_EXPLANATION,				// 説明画像
		REVERSI_BGM,						// BGM
		REVERSI_PIECE_SE,					// 駒SE
		REVERSI_CLEAR_SE,					// クリアSE
		REVERSI_OVER_SE,					// 敗北SE
		REVERSI_SKIP_SE,					// スキップSE

		BOARD,								// ボード
		QUORIDOR_BASE,
		QUORIDOR_DESK,
		QUORIDOR_GRID,
		QUORIDOR_PIECE,
		QUORIDOR_TRIANGLE,
		QUORIDOR_WALL,
		QUORIDOR_TEXTURE_WHITE,
		QUORIDOR_PIECEMOVE_SE,
		QUORIDOR_WALLMOVE_SE,
		QUORIDOR_WALLROTATION_SE,
		QUORIDOR_MODECHANGE_SE,
		QUORIDOR_DICIDE_SE,
		QUORIDOR_VICTORY_SE,


		MINISHOGI_TATAMI,
		MINISHOGI_SHOGIBAN,
		MINISHOGI_KOMADAI,
		MINISHOGI_FU,
		MINISHOGI_GIN,
		MINISHOGI_KIN,
		MINISHOGI_KAKU,
		MINISHOGI_HISHA,
		MINISHOGI_OU,
		MINISHOGI_GYOKU,
		MINISHOGI_CURSOR,
		MINISHOGI_TEXTURE_UV
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
