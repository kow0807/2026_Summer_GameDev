#include "../../Manager/ResourceManager.h"
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
    cpuPressed_(false)
{
}

FirstPressGame::~FirstPressGame(void)
{
}

void FirstPressGame::Init(void)
{
    waitTime_ = 120 + GetRand(600);
    timer_ = 0;
    isPressed_ = false;
    cpuPressed_ = false;
}

void FirstPressGame::Update(void)
{
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
        // CPUの反応時間決定
        if (timer_ == 1)
        {
            cpuPressed_ = false;
            cpuPressFrame_ = 10 + GetRand(60); // CPUの反応速度
        }

        if (timer_ > waitTime_)
        {
            gameState_ = GameState::GO;
            timer_ = 0;
        }

        // フライング（プレイヤー）
        if (CheckHitKey(KEY_INPUT_A))
        {
            gameState_ = GameState::RESULT;
            pressFrame_ = -1;
        }
        break;

    case GameState::GO:
        // プレイヤー入力
        if (CheckHitKey(KEY_INPUT_A) && !isPressed_)
        {
            isPressed_ = true;
            pressFrame_ = timer_;
        }

        // CPU入力
        if (!cpuPressed_ && timer_ >= cpuPressFrame_)
        {
            cpuPressed_ = true;
        }

        // 勝敗確定（どちらか押したら終了）
        if (isPressed_ || cpuPressed_)
        {
            gameState_ = GameState::RESULT;
        }
        break;

    case GameState::RESULT:
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
    int bg = GetColor(20, 20, 40);

    int centerX = screenX / 2;

    // 背景
    DrawBox(0, 0, screenX, screenY, bg, TRUE);

    // 中央ライン（対戦感）
    DrawLine(centerX, 0, centerX, screenY, white);

    // 外枠
    DrawBox(10, 10, screenX - 10, screenY - 10, white, FALSE);

    // タイトル
    DrawString(centerX - 120, 20, "早押し勝負", white);

    // プレイヤー / CPU ラベル
    DrawString(centerX / 2 - 40, 80, "PLAYER", green);
    DrawString(centerX + centerX / 2 - 30, 80, "CPU", red);

    // 状態表示（中央）
    switch (gameState_)
    {
    case GameState::WAIT:
        DrawString(centerX - 180, screenY / 2, "「押せ！！」が出たらAを押せ！", white);
        break;

    case GameState::READY:
        DrawString(centerX - 40, screenY / 2, "READY...", yellow);
        break;

    case GameState::GO:
        DrawString(centerX - 40, screenY / 2, "PUSH!!", red);
        break;

    case GameState::RESULT:
    {
        char buf[64];

        // 左（プレイヤー）
        if (pressFrame_ < 0)
        {
            DrawString(centerX / 2 - 80, screenY / 2, "FLYING!", red);
        }
        else if (isPressed_)
        {
            sprintf_s(buf, "TIME: %d", pressFrame_);
            DrawString(centerX / 2 - 80, screenY / 2, buf, green);
        }

        // 右（CPU）
        if (cpuPressed_)
        {
            sprintf_s(buf, "TIME: %d", cpuPressFrame_);
            DrawString(centerX + centerX / 2 - 80, screenY / 2, buf, red);
        }

        // 勝敗（下中央）
        if (pressFrame_ < 0)
        {
            DrawString(centerX - 60, screenY - 80, "YOU LOSE", red);
        }
        else if (!isPressed_)
        {
            DrawString(centerX - 60, screenY - 80, "CPU WIN", red);
        }
        else if (!cpuPressed_ || pressFrame_ < cpuPressFrame_)
        {
            DrawString(centerX - 60, screenY - 80, "YOU WIN!", green);
        }
        else
        {
            DrawString(centerX - 60, screenY - 80, "CPU WIN", red);
        }
    }
    break;
    }

    // 押下状態の軽い演出
    if (isPressed_)
    {
        DrawString(centerX / 2 - 30, screenY / 2 + 40, "PUSH!", green);
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
    gameState_ = GameState::WAIT;
    timer_ = 0;
    isPressed_ = false;
    cpuPressed_ = false;
}