#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/Setting.h"
#include "../../Application.h"
#include "ButtonMashGame.h"

ButtonMashGame::ButtonMashGame(void)
{
}

ButtonMashGame::~ButtonMashGame(void)
{
}

void ButtonMashGame::Init(void)
{
    backImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_BACK).handleId_;
    backUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_BACK_UI).handleId_;
    readyUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_READY_UI).handleId_;
    pressUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_PRESS_UI).handleId_;
    winUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_WIN_UI).handleId_;
    loseUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_LOSE_UI).handleId_;
    countUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_COUNT_UI).handleId_;
    winCountUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_WIN_COUNT_UI).handleId_;
    countUI2Img_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_COUNT_UI_2).handleId_;
    loseCountUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_LOSE_COUNT_UI).handleId_;
    pointUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_POINT_UI).handleId_;
    lostUIImg_ = resMng_.Load(ResourceManager::SRC::BUTTON_MASH_GAME_LOST_UI).handleId_;

    playerWinCount_ = 0;
    cpuWinCount_ = 0;
    isReturn_ = false;
    Reset();
}

void ButtonMashGame::Reset(void)
{
    gameState_ = GameState::READY;

    battleRate_ = 0.5f;

    readyTimer_ = 180;

    resultTimer_ = 180;

    isPlayerWin_ = false;

    playerFlashPower_ = 0.0f;
    cpuFlashPower_ = 0.0f;
}

void ButtonMashGame::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    switch (gameState_)
    {
    case GameState::READY:

        readyTimer_--;

        if (readyTimer_ <= 0)
        {
            gameState_ = GameState::GO;
        }

        break;

    case GameState::GO:

        // PLAYER連打
        if (ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
        {
            battleRate_ += 0.03f;

            // PLAYER側発光
            playerFlashPower_ = 1.0f;
        }

        // CPU連打
        if (GetRand(100) < 20)
        {
            battleRate_ -= 0.02f;

            // CPU側発光
            cpuFlashPower_ = 1.0f;
        }

        // 徐々に中央へ戻る
        battleRate_ += (0.5f - battleRate_) * 0.0001f;

        // 制限
        if (battleRate_ < -0.03f)
        {
            battleRate_ = -0.03f;
        }

        if (battleRate_ > 1.03f)
        {
            battleRate_ = 1.03f;
        }

        // PLAYER勝利
        if (battleRate_ >= 1.03f)
        {
            gameState_ = GameState::RESULT;

            playerWinCount_++;

            isPlayerWin_ = true;
        }

        // CPU勝利
        if (battleRate_ <= -0.03f)
        {
            gameState_ = GameState::RESULT;

            cpuWinCount_++;

            isPlayerWin_ = false;
        }

        break;

    case GameState::RESULT:

        resultTimer_--;

        if (resultTimer_ <= 0)
        {
            if (playerWinCount_ >= 2 || cpuWinCount_ >= 2)
            {
                // 選択画面に戻る
                isReturn_ = true;
            }
            else
            {
                Reset();
            }
        }

        break;
    }

    // 発光減衰
    playerFlashPower_ *= 0.90f;
    cpuFlashPower_ *= 0.90f;

    if (playerFlashPower_ < 0.01f)
    {
        playerFlashPower_ = 0.0f;
    }

    if (cpuFlashPower_ < 0.01f)
    {
        cpuFlashPower_ = 0.0f;
    }
}

void ButtonMashGame::Draw(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    // 背景
    DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);

    // 境界位置
    int borderX = (int)(screenX * battleRate_);

    // エリア描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

    DrawBattleArea(borderX);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 境界ライン
    DrawBattleLine(borderX);
}

void ButtonMashGame::DrawUI(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    DrawRotaGraph(centerX, centerY, 0.6, 0.0, backUIImg_, true);
    DrawCountUI();

    switch (gameState_)
    {
    case GameState::READY:
        DrawRotaGraph(centerX, centerY, 0.5, 0.0, readyUIImg_, true);
        break;

    case GameState::GO:
        DrawRotaGraph(centerX, centerY, 0.5, 0.0, pressUIImg_, true);
        break;

    case GameState::RESULT:
        if (playerWinCount_ >= 2)
        {
            DrawRotaGraph(centerX, centerY, 0.7, 0.0, winUIImg_, true);
        }
        else if (cpuWinCount_ >= 2)
        {
            DrawRotaGraph(centerX, centerY, 0.7, 0.0, loseUIImg_, true);
        }
        else
        {
            if (isPlayerWin_)
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, pointUIImg_, true);
            }
            else
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, lostUIImg_, true);
            }
        }
        break;
    }
}

void ButtonMashGame::DrawBattleLine(int borderX)
{
    int screenY = Application::SCREEN_SIZE_Y;

    int prevX = borderX + GetBattleOffset(0);
    int prevY = 0;

    for (int y = 16; y <= screenY; y += 16)
    {
        int currentX = borderX + GetBattleOffset(y);

        // 外側グロー
        SetDrawBlendMode(DX_BLENDMODE_ADD, 40);

        for (int i = 10; i >= 4; i -= 2)
        {
            DrawLine(
                prevX - i,
                prevY,
                currentX - i,
                y,
                GetColor(0, 255, 255)
            );

            DrawLine(
                prevX + i,
                prevY,
                currentX + i,
                y,
                GetColor(0, 255, 255)
            );
        }

        // 中間発光
        SetDrawBlendMode(DX_BLENDMODE_ADD, 120);

        for (int i = 2; i >= 1; i--)
        {
            DrawLine(
                prevX - i,
                prevY,
                currentX - i,
                y,
                GetColor(120, 255, 255)
            );

            DrawLine(
                prevX + i,
                prevY,
                currentX + i,
                y,
                GetColor(120, 255, 255)
            );
        }

        // コアライン
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);

        DrawLine(
            prevX,
            prevY,
            currentX,
            y,
            GetColor(255, 255, 255)
        );

        // 稲妻枝
        if ((y + GetNowCount()) % 48 < 6)
        {
            int branchX = currentX + ((y * 7) % 30 - 15);
            int branchY = y + 20;

            SetDrawBlendMode(DX_BLENDMODE_ADD, 160);

            DrawLine(
                currentX,
                y,
                branchX,
                branchY,
                GetColor(150, 255, 255)
            );
        }

        prevX = currentX;
        prevY = y;
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void ButtonMashGame::DrawBattleArea(int borderX)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    // PLAYERカラー
    int playerBlue =
        255;

    int playerGreen =
        180 + (int)(playerFlashPower_ * 75);

    if (playerGreen > 255)
    {
        playerGreen = 255;
    }

    int playerColor =
        GetColor(0, playerGreen, playerBlue);

    // CPUカラー
    int cpuRed =
        238 + (int)(cpuFlashPower_ * 17);

    int cpuBlue =
        238 + (int)(cpuFlashPower_ * 17);

    if (cpuRed > 255)
    {
        cpuRed = 255;
    }

    if (cpuBlue > 255)
    {
        cpuBlue = 255;
    }

    int cpuColor =
        GetColor(cpuRed, 130, cpuBlue);

    int prevX = borderX + GetBattleOffset(0);
    int prevY = 0;

    for (int y = 16; y <= screenY; y += 16)
    {
        int currentX = borderX + GetBattleOffset(y);

        // PLAYER側
        DrawTriangle(
            0,
            prevY,
            prevX,
            prevY,
            currentX,
            y,
            playerColor,
            true
        );

        DrawTriangle(
            0,
            prevY,
            0,
            y,
            currentX,
            y,
            playerColor,
            true
        );

        // CPU側
        DrawTriangle(
            screenX,
            prevY,
            prevX,
            prevY,
            currentX,
            y,
            cpuColor,
            true
        );

        DrawTriangle(
            screenX,
            prevY,
            screenX,
            y,
            currentX,
            y,
            cpuColor,
            true
        );

        prevX = currentX;
        prevY = y;
    }

    // PLAYER発光
    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        (int)(playerFlashPower_ * 30)
    );

    DrawBox(
        0,
        0,
        borderX,
        screenY,
        GetColor(0, 255, 255),
        true
    );

    // CPU発光
    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        (int)(cpuFlashPower_ * 30)
    );

    DrawBox(
        borderX,
        0,
        screenX,
        screenY,
        GetColor(255, 100, 255),
        true
    );

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

int ButtonMashGame::GetBattleOffset(int y)
{
    int offsetX =
        (int)(sin((y + GetNowCount() * 0.25f) * 0.04f) * 10.0f);

    offsetX += ((y * 13 + GetNowCount() / 2) % 7) - 3;

    return offsetX;
}

void ButtonMashGame::DrawCountUI(void)
{
    // プレイヤー側
    if (playerWinCount_ <= 0)
    {
        DrawRotaGraph(150, 150, 0.2, 0.0, countUIImg_, true);
        DrawRotaGraph(200, 150, 0.2, 0.0, countUIImg_, true);
    }
    else if (playerWinCount_ == 1)
    {
        DrawRotaGraph(150, 150, 0.2, 0.0, winCountUIImg_, true);
        DrawRotaGraph(200, 150, 0.2, 0.0, countUIImg_, true);
    }
    else if (playerWinCount_ == 2)
    {
        DrawRotaGraph(150, 150, 0.2, 0.0, winCountUIImg_, true);
        DrawRotaGraph(200, 150, 0.2, 0.0, winCountUIImg_, true);
    }

    // CPU側
    if (cpuWinCount_ <= 0)
    {
        DrawRotaGraph(880, 150, 0.2, 0.0, countUI2Img_, true);
        DrawRotaGraph(830, 150, 0.2, 0.0, countUI2Img_, true);
    }
    else if (cpuWinCount_ == 1)
    {
        DrawRotaGraph(880, 150, 0.2, 0.0, loseCountUIImg_, true);
        DrawRotaGraph(830, 150, 0.2, 0.0, countUI2Img_, true);
    }
    else if (cpuWinCount_ == 2)
    {
        DrawRotaGraph(880, 150, 0.2, 0.0, loseCountUIImg_, true);
        DrawRotaGraph(830, 150, 0.2, 0.0, loseCountUIImg_, true);
    }
}