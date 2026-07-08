#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
{

}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// íËì_ÉJÉÅÉâ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

	backImg_ = resMng_.Load(ResourceManager::SRC::TITLE_BACK).handleId_;
    logoImg_ = resMng_.Load(ResourceManager::SRC::TITLE_LOGO).handleId_;
    isBgm_ = true;
    bgm_ = resMng_.Load(ResourceManager::SRC::TITLE_BGM).handleId_;
    startSe_ = resMng_.Load(ResourceManager::SRC::TITLE_START_SE).handleId_;

    uiFont_ = CreateFontToHandle(
        "ü‡ñæí©",
        28,
        6);
}

void TitleScene::Update(void)
{
    if (isBgm_)
    {
        PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
        isBgm_ = false;
    }
	InputManager& ins = InputManager::GetInstance();

	if(ins.IsNew(KEY_INPUT_SPACE) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
        PlaySoundMem(startSe_, DX_PLAYTYPE_BACK);
        StopSoundMem(bgm_);
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void TitleScene::Draw(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    // îwåi
    DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);

    // ÉçÉS
    DrawRotaGraph(centerX, centerY, 0.4, 0.0, logoImg_, true);

    // PUSH SPACE
    if ((GetNowCount() / 500) % 2 == 0)
    {
        const char* text = "PUSH SPACE";

        int textWidth = GetDrawStringWidthToHandle(
            text,
            strlen(text),
            uiFont_);

        DrawStringToHandle(
            centerX - textWidth / 2,
            screenY - 160,
            text,
            GetColor(0, 0, 0),
            uiFont_);
    }
}
void TitleScene::DrawUI(void)
{

}
