#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
    mSelectIndex_(0)
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{

	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);
}

void GameScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	// いったん
 	if(ins.IsNew(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		printfDx("通過\n");
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	
	}

    SelectGameUpdate();
}

void GameScene::Draw(void)
{

}

void GameScene::DrawUI(void)
{
	DrawFormatString(Application::SCREEN_SIZE_X / 2 - 100, Application::SCREEN_SIZE_Y / 2 - 100, GetColor(255, 255, 255), "ミニゲーム選ぶぜ！！");
    SelectGameDrawUI();
}

void GameScene::SelectGameUpdate(void)
{
    InputManager& ins = InputManager::GetInstance();

    // ミニゲーム選択
    // 右キー
    if (ins.IsTrgDown(KEY_INPUT_RIGHT))
    {
        mSelectIndex_++;
        if (mSelectIndex_ >= M_MAX_SELECT)
        {
            mSelectIndex_ = 0; // ループ
        }
    }

    // 左キー
    if (ins.IsTrgDown(KEY_INPUT_LEFT))
    {
        mSelectIndex_--;
        if (mSelectIndex_ < 0)
        {
            mSelectIndex_ = M_MAX_SELECT - 1; // ループ
        }
    }

    // 決定
    if (ins.IsTrgDown(KEY_INPUT_RETURN))
    {
        printfDx("選択: %d\n", mSelectIndex_);

        // 選択に応じて変更
        switch (mSelectIndex_)
        {
        case 0:
            // 別ミニゲーム
            break;
        case 1:
            // 別ミニゲーム
            break;
        case 2:
            // 別ミニゲーム
            break;
        case 3:
            // 別ミニゲーム
            break;
        case 4:
            // 別ミニゲーム
            break;
        case 5:
            // 別ミニゲーム
            break;
        case 6:
            // 別ミニゲーム
            break;
        case 7:
            // 別ミニゲーム
            break;
        }
    }
}

void GameScene::SelectGameDrawUI(void)
{
    const int centerX = Application::SCREEN_SIZE_X / 2;
    const int centerY = Application::SCREEN_SIZE_Y / 2;

    const int cardW = 200;
    const int cardH = 120;
    const int spacing = 260;

    const char* names[M_MAX_SELECT] =
    {
        "早押し",
        "四択クイズ",
        "オセロ",
        "連打対決",
        "フラッシュ暗算",
        "コリドール",
        "ウサギと猟犬",
        "5五将棋"
    };

    for (int i = 0; i < M_MAX_SELECT; i++)
    {
        int diff = i - mSelectIndex_;

        // ループ補正
        if (diff > M_MAX_SELECT / 2)
            diff -= M_MAX_SELECT;
        if (diff < -M_MAX_SELECT / 2)
            diff += M_MAX_SELECT;

        int x = centerX + diff * spacing;
        int y = centerY;

        // 選択中判定
        bool isSelect = (i == mSelectIndex_);

        int color = isSelect ? GetColor(255, 255, 0) : GetColor(200, 200, 200);

        // カード
        DrawBox(x - cardW / 2, y - cardH / 2,
            x + cardW / 2, y + cardH / 2,
            color, FALSE);

        if (isSelect)
        {
            DrawBox(x - cardW / 2, y - cardH / 2,
                x + cardW / 2, y + cardH / 2,
                GetColor(255, 255, 0), TRUE);
        }

        // 名前
        int textW = GetDrawStringWidth(names[i], static_cast<int>(strlen(names[i])));
        DrawString(x - textW / 2, y + cardH / 2 + 10,
            names[i], GetColor(255, 255, 255));
    }
}