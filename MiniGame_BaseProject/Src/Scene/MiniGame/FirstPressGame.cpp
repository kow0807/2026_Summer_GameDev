#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/Setting.h"
#include "../../Application.h"
#include "FirstPressGame.h"

FirstPressGame::FirstPressGame(void)
    :
    gameState_(GameState::WAIT),
    timer_(0),
    waitTime_(180),
    isPressed_(false),
    pressFrame_(0),
    cpuPressFrame_(0),
    cpuPressed_(false),
    round_(1),
    playerWin_(0),
    cpuWin_(0),
    readyAlpha_(255),
    isFlash_(false)
{
}

FirstPressGame::~FirstPressGame(void)
{
}

void FirstPressGame::Init(void)
{
    round_ = 1;
    playerWin_ = 0;
    cpuWin_ = 0;
    readyAlpha_ = 255;
    isFlash_ = false;

    backImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_BACK).handleId_;
    backUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_BACK_UI).handleId_;
    readyUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_READY_UI).handleId_;
    pressUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_PRESS_UI).handleId_;
    winUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_WIN_UI).handleId_;
    loseUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_LOSE_UI).handleId_;
    countUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_COUNT_UI).handleId_;
    winCountUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_WIN_COUNT_UI).handleId_;
    countUI2Img_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_COUNT_UI_2).handleId_;
    loseCountUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_LOSE_COUNT_UI).handleId_;
    pointUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_POINT_UI).handleId_;
    lostUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_LOST_UI).handleId_;
    playerTimeUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_PLAYER_TIME_UI).handleId_;

    ResetRound();
}

void FirstPressGame::ResetRound(void)
{
    waitTime_ = 120 + GetRand(600);
    timer_ = 0;
    isPressed_ = false;
    cpuPressed_ = false;
    pressFrame_ = 0;
    cpuPressFrame_ = 0;
    readyAlpha_ = 255;
    gameState_ = GameState::WAIT;
}

void FirstPressGame::DrawFadeReady(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    // 透明度付き描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, readyAlpha_);

    DrawRotaGraph(centerX, centerY, 0.5, 0.0, readyUIImg_, true);

    // 描画設定を戻す
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 徐々に透明にする
    if (readyAlpha_ > 0)
    {
        readyAlpha_ -= 2;

        if (readyAlpha_ < 0)
        {
            readyAlpha_ = 0;
        }
    }
}

void FirstPressGame::DrawCountUI(void)
{
    // プレイヤー側
    if (playerWin_ <= 0)
    {
        DrawRotaGraph(150, 150, 0.2, 0.0, countUIImg_, true);
        DrawRotaGraph(200, 150, 0.2, 0.0, countUIImg_, true);
    }
    else if (playerWin_ == 1)
    {
        DrawRotaGraph(150, 150, 0.2, 0.0, winCountUIImg_, true);
        DrawRotaGraph(200, 150, 0.2, 0.0, countUIImg_, true);
    }
    else if (playerWin_ == 2)
    {
        DrawRotaGraph(150, 150, 0.2, 0.0, winCountUIImg_, true);
        DrawRotaGraph(200, 150, 0.2, 0.0, winCountUIImg_, true);
    }

    // CPU側
    if (cpuWin_ <= 0)
    {
        DrawRotaGraph(880, 150, 0.2, 0.0, countUI2Img_, true);
        DrawRotaGraph(830, 150, 0.2, 0.0, countUI2Img_, true);
    }
    else if (cpuWin_ == 1)
    {
        DrawRotaGraph(880, 150, 0.2, 0.0, loseCountUIImg_, true);
        DrawRotaGraph(830, 150, 0.2, 0.0, countUI2Img_, true);
    }
    else if (cpuWin_ == 2)
    {
        DrawRotaGraph(880, 150, 0.2, 0.0, loseCountUIImg_, true);
        DrawRotaGraph(830, 150, 0.2, 0.0, loseCountUIImg_, true);
    }
}

void FirstPressGame::Flash(void)
{
    // 呼び出しごとに管理する用
    static int alpha = 255;

    // フラッシュ中だけ描画
    if (isFlash_)
    {
        // 白色を半透明で全画面描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        DrawBox(
            0,
            0,
            Application::SCREEN_SIZE_X,
            Application::SCREEN_SIZE_Y,
            GetColor(255, 255, 255),
            TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 徐々に透明化
        alpha -= 10;

        // 完了
        if (alpha <= 0)
        {
            alpha = 255;
            isFlash_ = false;
        }
    }
}

void FirstPressGame::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    timer_++;

    switch (gameState_)
    {
    case GameState::WAIT:
        if (timer_ > 60)
        {
            gameState_ = GameState::READY;
            timer_ = 0;
        }
        break;

    case GameState::READY:
        if (timer_ == 1)
        {
            cpuPressed_ = false;
            cpuPressFrame_ = 10 + GetRand(60);
        }

        if (timer_ > waitTime_)
        {
            gameState_ = GameState::GO;
            timer_ = 0;
        }

        // フライング
        if (ins.IsTrgDown(KEY_INPUT_A))
        {
            isFlash_ = true;
            pressFrame_ = -1;
            cpuWin_++;
            gameState_ = GameState::RESULT;
        }
        break;

    case GameState::GO:
        // プレイヤー
        if (ins.IsTrgDown(KEY_INPUT_A) && !isPressed_)
        {
            isFlash_ = true;
            isPressed_ = true;
            pressFrame_ = timer_;
        }

        // CPU
        if (!cpuPressed_ && timer_ >= cpuPressFrame_)
        {
            isFlash_ = true;
            cpuPressed_ = true;
        }

        if (isPressed_ || cpuPressed_)
        {
            isFlash_ = true;
            gameState_ = GameState::RESULT;
            timer_ = 0;
        }
        break;

    case GameState::RESULT:
        // 勝敗判定
        if (timer_ == 1)
        {
            if (pressFrame_ < 0)
            {
                cpuWin_++;
            }
            else if (!isPressed_)
            {
                cpuWin_++;
            }
            else if (!cpuPressed_ || pressFrame_ < cpuPressFrame_)
            {
                playerWin_++;
            }
            else
            {
                cpuWin_++;
            }
        }

        // 少し待って次ラウンドへ
        if (timer_ > 120)
        {
            // 2勝したら終了
            if (playerWin_ >= 2 || cpuWin_ >= 2 || round_ >= 3)
            {
                // 完全終了（ここでは止めるだけ）
            }
            else
            {
                round_++;
                ResetRound();
            }
        }
        break;
    }
}

void FirstPressGame::Draw(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int white = GetColor(255, 255, 255);
    int red = GetColor(255, 60, 60);
    int green = GetColor(60, 255, 60);
    int yellow = GetColor(255, 255, 0);

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    // 背景
    DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);
    DrawRotaGraph(centerX, centerY, 0.6, 0.0, backUIImg_, true);

    // カウント
    DrawCountUI();

    switch (gameState_)
    {
    case GameState::WAIT:
        DrawRotaGraph(centerX, centerY, 0.5, 0.0, readyUIImg_, true);
        break;

    case GameState::READY:
        DrawFadeReady();
        break;

    case GameState::GO:
        DrawRotaGraph(centerX, centerY, 0.5, 0.0, pressUIImg_, true);
        break;

    case GameState::RESULT:
    {
        char buf[64];

        if (pressFrame_ < 0)
        {
            DrawString(centerX / 2 - 80, screenY / 2, "FLYING!", red);
        }
        else if (isPressed_)
        {
            DrawRotaGraph(centerX / 2, screenY / 2, 0.25, 0.0, playerTimeUIImg_, true);
            sprintf_s(buf, "TIME: %d", pressFrame_);
            DrawString(centerX / 2 - 80, screenY / 2 - 20, buf, green);
        }

        if (cpuPressed_)
        {
            sprintf_s(buf, "TIME: %d", cpuPressFrame_);
            DrawString(centerX + centerX / 2 - 80, screenY / 2, buf, red);
        }

        // 勝敗表示
        if (playerWin_ < 2 && cpuWin_ < 2)
        {
            if (pressFrame_ < 0)
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, lostUIImg_, true);
            }
            else if (!isPressed_)
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, lostUIImg_, true);
            }
            else if (!cpuPressed_ || pressFrame_ < cpuPressFrame_)
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, pointUIImg_, true);
            }
            else
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, lostUIImg_, true);
            }
        }

        // 最終結果
        if (playerWin_ >= 2 || cpuWin_ >= 2 || round_ >= 3)
        {
            if (playerWin_ > cpuWin_)
            {
                DrawRotaGraph(centerX, centerY, 0.7, 0.0, winUIImg_, true);
            }
            else
            {
                DrawRotaGraph(centerX, centerY, 0.6, 0.0, loseUIImg_, true);
            }
        }
        Flash();
    }
    break;
    }

    if (isPressed_)
    {
        //DrawString(centerX / 2 - 30, screenY / 2 + 40, "PUSH!", green);
    }
    if (cpuPressed_)
    {
        DrawString(centerX + centerX / 2 - 30, screenY / 2 + 40, "PUSH!", red);
    }
}

void FirstPressGame::DrawUI(void)
{
}

void FirstPressGame::Reset(void)
{
    Init();
}