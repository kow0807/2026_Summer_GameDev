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

	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

	backImg_ = resMng_.Load(ResourceManager::SRC::TITLE_BACK).handleId_;
    logoImg_ = resMng_.Load(ResourceManager::SRC::TITLE_LOGO).handleId_;
    isBgm_ = true;
    bgm_ = resMng_.Load(ResourceManager::SRC::TITLE_BGM).handleId_;
    startSe_ = resMng_.Load(ResourceManager::SRC::TITLE_START_SE).handleId_;
    moveSe_ = resMng_.Load(ResourceManager::SRC::TITLE_MOVE_SE).handleId_;

    uiFont_ = CreateFontToHandle(
        "游明朝",
        40,
        16);

    selectNo_ = 0;
}

void TitleScene::Update(void)
{
    if (isBgm_)
    {
        PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
        isBgm_ = false;
    }

    InputManager& ins = InputManager::GetInstance();

    //--------------------------------------
    // カーソル移動
    //--------------------------------------
    if (ins.IsTrgDown(KEY_INPUT_UP) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP))
    {
        PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);
        selectNo_--;

        if (selectNo_ < 0)
            selectNo_ = 1;
    }

    if (ins.IsTrgDown(KEY_INPUT_DOWN) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))
    {
        PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);
        selectNo_++;

        if (selectNo_ > 1)
            selectNo_ = 0;
    }

    //--------------------------------------
    // 決定
    //--------------------------------------
    if (ins.IsTrgDown(KEY_INPUT_RETURN) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
    {
        PlaySoundMem(startSe_, DX_PLAYTYPE_BACK);

        switch (selectNo_)
        {
        case 0: // Game Start
            StopSoundMem(bgm_);
            SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
            break;

        case 1: // Exit
            SceneManager::GetInstance().SetGameEnd(true);
            break;
        }
    }
}

void TitleScene::Draw(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    //--------------------------------------
    // 背景
    //--------------------------------------
    DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);

    //--------------------------------------
    // ロゴ
    //--------------------------------------
    DrawRotaGraph(centerX, centerY - 120, 0.4, 0.0, logoImg_, true);

    //--------------------------------------
    // メニュー
    //--------------------------------------
    const char* menu[2] =
    {
        "Game Start",
        "Exit"
    };

    int menuStartY = centerY + 80;
    int time = GetNowCount();

    for (int i = 0; i < 2; i++)
    {
        bool isSelect = (i == selectNo_);

        int offsetY = 0;
        double scale = 1.0;
        int arrowOffsetX = 0;

        if (isSelect)
        {
            // ゆっくり1px上下
            offsetY = static_cast<int>(sin(time * 0.005) * 1);

            // 少しだけ大きく
            scale = 1.03;

            // 矢印だけ少し左右に動く
            arrowOffsetX = static_cast<int>(sin(time * 0.008) * 2);
        }

        int width = GetDrawStringWidthToHandle(
            menu[i],
            static_cast<int>(strlen(menu[i])),
            uiFont_);

        int x = centerX - width / 2;
        int y = menuStartY + i * 70 + offsetY;

        if (isSelect)
        {
            DrawExtendStringToHandle(
                x,
                y,
                scale,
                scale,
                menu[i],
                GetColor(0, 0, 0),
                uiFont_);

            DrawStringToHandle(
                x - 45 + arrowOffsetX,
                y,
                ">",
                GetColor(0, 0, 0),
                uiFont_);
        }
        else
        {
            DrawStringToHandle(
                x,
                y,
                menu[i],
                GetColor(140, 140, 140),
                uiFont_);
        }
    }
}

void TitleScene::DrawUI(void)
{

}
