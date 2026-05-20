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

	// プレイヤー時間UI
	res = std::make_shared<RES>(RES_T::IMG, PATH_IMG + "FirstPressGame/Player_Time_UI.png");
	resourcesMap_.emplace(SRC::FIRST_PRESS_GAME_PLAYER_TIME_UI, res);

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
