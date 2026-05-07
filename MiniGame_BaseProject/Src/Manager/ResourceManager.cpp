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

	// 板
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Board/Board.mv1");
	resourcesMap_.emplace(SRC::BOARD, res);

	// 板のテクスチャ
	res = std::make_shared<RES>(RES_T::IMG, PATH_MDL + "Board/texture/WoodFloor_4K.jpg");
	resourcesMap_.emplace(SRC::WOOD_BOARD_TEXTURE, res);

	// 板のテクスチャ(ノーマルマップ)
	res = std::make_shared<RES>(RES_T::IMG, PATH_MDL + "Board/texture/WoodFloor_4K_N.jpg");
	resourcesMap_.emplace(SRC::WOOD_BOARD_TEXTURE_N, res);

	// 木の壁
	res = std::make_shared<RES>(RES_T::MODEL, PATH_MDL + "Wall/Wall.mv1");
	resourcesMap_.emplace(SRC::WOOD_WALL, res);

	// 木の壁のテクスチャ
	res = std::make_shared<RES>(RES_T::IMG, PATH_MDL + "Wall/texture/WoodWall_Color.jpg");
	resourcesMap_.emplace(SRC::WOOD_WALL_TEXTURE, res);

	// 木の壁のテクスチャ（ノーマルマップ)
	res = std::make_shared<RES>(RES_T::IMG, PATH_MDL + "Wall/texture/WoodWall_4K_NormalDX.jpg");
	resourcesMap_.emplace(SRC::WOOD_WALL_TEXTURE_N, res);

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
