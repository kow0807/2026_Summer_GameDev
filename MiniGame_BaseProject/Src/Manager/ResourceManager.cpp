#include <DxLib.h>
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	std::shared_ptr<Resource> res;

	// タイトル画像
	//res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "Title.png");
	//resourcesMap_.emplace(SRC::TITLE, res);

	// 背景
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Back.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_BACK, res);

	// 背景UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Back_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_BACK_UI, res);

	// 準備UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Ready_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_READY_UI, res);

	// 押せ！UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Press_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_PRESS_UI, res);

	// 勝利UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Win_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_WIN_UI, res);

	// 敗北UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Lose_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_LOSE_UI, res);
	
	// カウントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Count_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_COUNT_UI, res);

	// 勝利カウントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Win_Count_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_WIN_COUNT_UI, res);
	
	// カウントUI2
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Count_UI_2.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_COUNT_UI_2, res);

	// 敗北カウントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Lose_Count_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_LOSE_COUNT_UI, res);

	// ポイントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Point_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_POINT_UI, res);

	// ロストUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Lost_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_LOST_UI, res);

	// フライングUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Flying_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_FLYING_UI, res);

	// 説明画像
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Explanation.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_EXPLANATION, res);

	// BGM
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Bgm.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_BGM, res);

	// Press音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Press.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_PRESS_SE, res);

	// Point音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Point.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_POINT_SE, res);

	// Lost音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Lost.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_LOST_SE, res);

	// クリア音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Clear.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_CLEAR_SE, res);

	// 敗北音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Over.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_OVER_SE, res);

	// 落下音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Falling.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_FALLING_SE, res);

	// ノイズ音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Noise.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_NOISE_SE, res);

	// カウント音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Count.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_COUNT_SE, res);

	// エラー音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Error.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_ERROR_SE, res);

	// トリビア音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "FirstPressGame/Trivia.mp3");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_TRIVIA_SE, res);



	// 背景
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Back.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_BACK, res);

	// 背景UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Back_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_BACK_UI, res);

	// 準備UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Ready_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_READY_UI, res);

	// 押せ！UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Press_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_PRESS_UI, res);

	// 勝利UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Win_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_WIN_UI, res);

	// 敗北UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Lose_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_LOSE_UI, res);

	// カウントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Count_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_COUNT_UI, res);

	// 勝利カウントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Win_Count_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_WIN_COUNT_UI, res);

	// カウントUI2
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Count_UI_2.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_COUNT_UI_2, res);

	// 敗北カウントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Lose_Count_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_LOSE_COUNT_UI, res);

	// ポイントUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Point_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_POINT_UI, res);

	// ロストUI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Lost_UI.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_LOST_UI, res);

	// 説明画像
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "ButtonMashGame/Explanation.png");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_EXPLANATION, res);

	// Ready音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Ready.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_READY_SE, res);

	// BGM
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Bgm.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_BGM, res);

	// Push音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Push.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_PUSH_SE, res);

	// クリア音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Clear.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_CLEAR_SE, res);

	// 敗北音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Over.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_OVER_SE, res);

	// ポイント音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Point.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_POINT_SE, res);

	// ロスト音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "ButtonMashGame/Lost.mp3");
	resourcesMap_.emplace(SRC::BUTTON_MASH_GAME_LOST_SE, res);



	// 背景
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "QuizGame/Back.png");
	resourcesMap_.emplace(SRC::QUIZ_GAME_BACK, res);

	// 説明画像
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "QuizGame/Explanation.png");
	resourcesMap_.emplace(SRC::QUIZ_GAME_EXPLANATION, res);

	// Ready音
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "QuizGame/Ready.mp3");
	resourcesMap_.emplace(SRC::QUIZ_GAME_READY_SE, res);

	// BGM
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "QuizGame/Bgm.mp3");
	resourcesMap_.emplace(SRC::QUIZ_GAME_BGM, res);

	// 正解
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "QuizGame/Correct.mp3");
	resourcesMap_.emplace(SRC::QUIZ_GAME_CORRECT, res);

	// 不正解
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "QuizGame/Buzzer.mp3");
	resourcesMap_.emplace(SRC::QUIZ_GAME_BUZZER, res);

	// カーソル
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "QuizGame/Cursor.mp3");
	resourcesMap_.emplace(SRC::QUIZ_GAME_CURSOR, res);

	// 結果
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "QuizGame/Result.mp3");
	resourcesMap_.emplace(SRC::QUIZ_GAME_RESULT, res);



	// 背景
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "Reversi/Back.png");
	resourcesMap_.emplace(SRC::REVERSI_BACK, res);

	// 説明画像
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "Reversi/Explanation.png");
	resourcesMap_.emplace(SRC::REVERSI_EXPLANATION, res);

	// BGM
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Reversi/Bgm.mp3");
	resourcesMap_.emplace(SRC::REVERSI_BGM, res);

	// 駒SE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Reversi/Piece.mp3");
	resourcesMap_.emplace(SRC::REVERSI_PIECE_SE, res);

	// クリアSE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Reversi/Clear.mp3");
	resourcesMap_.emplace(SRC::REVERSI_CLEAR_SE, res);

	// 敗北SE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Reversi/Over.mp3");
	resourcesMap_.emplace(SRC::REVERSI_OVER_SE, res);

	// スキップSE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Reversi/Skip.mp3");
	resourcesMap_.emplace(SRC::REVERSI_SKIP_SE, res);


	// 板
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Board/Board.mv1");
	resourcesMap_.emplace(SRC::BOARD, res);

	// クオリドールの盤面のモデル
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Quoridor/Base/Base.mv1");
	resourcesMap_.emplace(SRC::QUORIDOR_BASE, res);

	// クオリドールの机のモデル
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Quoridor/Desk/Desk.mv1");
	resourcesMap_.emplace(SRC::QUORIDOR_DESK, res);

	// クオリドールの盤面の升目のモデル
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Quoridor/Grid/Grid.mv1");
	resourcesMap_.emplace(SRC::QUORIDOR_GRID, res);

	// クオリドールの駒のモデル
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Quoridor/Piece/Piece.mv1");
	resourcesMap_.emplace(SRC::QUORIDOR_PIECE, res);

	// クオリドールの矢印のモデル
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Quoridor/Triangle/ETriangle.mv1");
	resourcesMap_.emplace(SRC::QUORIDOR_TRIANGLE, res);

	// クオリドールの壁のモデル
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Quoridor/Wall/Wall.mv1");
	resourcesMap_.emplace(SRC::QUORIDOR_WALL, res);

	// クオリドールの白いテクスチャ
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "Quoridor/Texture/White.jpg");
	resourcesMap_.emplace(SRC::QUORIDOR_TEXTURE_WHITE, res);

	// クオリドールのピースの移動SE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Quoridor/pieceMove.mp3");
	resourcesMap_.emplace(SRC::QUORIDOR_PIECEMOVE_SE, res);

	// クオリドールの壁移動
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Quoridor/wallMove.mp3");
	resourcesMap_.emplace(SRC::QUORIDOR_WALLMOVE_SE, res);

	// クオリドールの壁回転
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Quoridor/wallRotaion.mp3");
	resourcesMap_.emplace(SRC::QUORIDOR_WALLROTATION_SE, res);

	// クオリドールのモード切替SE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Quoridor/modeChange.mp3");
	resourcesMap_.emplace(SRC::QUORIDOR_MODECHANGE_SE, res);

	// クオリドールの決定SE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Quoridor/decide.mp3");
	resourcesMap_.emplace(SRC::QUORIDOR_DICIDE_SE, res);

	// クオリドールの勝利SE
	res = std::make_shared<RES>(RES_T::SOUND, PATH_SND + "Quoridor/victory.mp3");
	resourcesMap_.emplace(SRC::QUORIDOR_VICTORY_SE, res);

	// 五々将棋の畳
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi");
	resourcesMap_.emplace(SRC::MINISHOGI_TATAMI, res);

	// 五々将棋の将棋台
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Shogiban/Shogiban.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_SHOGIBAN, res);

	// 五々将棋の駒台
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Komadai/Komadai.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_KOMADAI, res);

	// 五々将棋の歩
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Fu/Fu.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_FU, res);

	// 五々将棋の銀
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Gin/Gin.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_GIN, res);

	// 五々将棋の金
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Kin/Kin.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_KIN, res);

	// 五々将棋の角
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Kakugyo/Kakugyo.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_KAKU, res);

	// 五々将棋の飛
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Hisha/Hisha.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_HISHA, res);

	// 五々将棋の王
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Ou/Ou.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_OU, res);

	// 五々将棋の玉
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "MiniShogi/Koma/Gyoku/Gyoku.mv1");
	resourcesMap_.emplace(SRC::MINISHOGI_GYOKU, res);

	// 五々将棋のUVテクスチャ
	res = std::make_unique<RES>(RES_T::IMG, PATH_MDL + "MiniShogi/Shogiban/Texture/Wood_2K_NormalDX.jpg");
	resourcesMap_.emplace(SRC::MINISHOGI_TEXTURE_UV, res);
}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	for (auto& res : resourcesMap_)
	{
		res.second->Release();
		//delete res.second;
	}
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{

	// ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return *resourcesMap_.find(src)->second;
	}

	// リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		// 登録されていない
		return dummy_;
	}

	// ロード処理
	rPair->second->Load();

	// 念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;

}
