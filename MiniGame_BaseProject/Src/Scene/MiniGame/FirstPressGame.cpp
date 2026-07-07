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
    isFlash_(false),
    isFall_(false),
    fallSpeed_(0.0f),
    angle_(0.0f),
    fallX_(0),
    fallY_(0),
    countDownFrame_(0)
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
    isFall_ = false;
    fallSpeed_ = 0.0f;
    angle_ = 0.0f;
    fallX_ = 0;
    fallY_ = 0;
    countDownFrame_ = 0;
    isReturn_ = false;

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
    flyingUIImg_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_FLYING_UI).handleId_;
    bgm_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_BGM).handleId_;
    pressSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_PRESS_SE).handleId_;
    pointSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_POINT_SE).handleId_;
    lostSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_LOST_SE).handleId_;
    clearSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_CLEAR_SE).handleId_;
    overSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_OVER_SE).handleId_;
    fallingSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_FALLING_SE).handleId_;
    noiseSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_NOISE_SE).handleId_;
    countSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_COUNT_SE).handleId_;
    errorSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_ERROR_SE).handleId_;
    ChangeVolumeSoundMem(160, errorSe_);
    triviaSe_ = resMng_.Load(ResourceManager::SRC::FIRST_PRESS_GAME_TRIVIA_SE).handleId_;
    ChangeVolumeSoundMem(210, triviaSe_);
    ResetRound();
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
            PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
            timer_ = 0;
        }
        break;

    case GameState::READY:
        FakeFalling();
        if (timer_ == 1)
        {
            cpuPressed_ = false;
            cpuPressFrame_ = 15 + GetRand(30);
        }

        if (timer_ > waitTime_)
        {
            StopSoundMem(bgm_);
            StopSoundMem(fallingSe_);
            StopSoundMem(noiseSe_);
            StopSoundMem(countSe_);
            StopSoundMem(errorSe_);
            StopSoundMem(triviaSe_);
            PlaySoundMem(pressSe_, DX_PLAYTYPE_BACK);
            gameState_ = GameState::GO;
            timer_ = 0;
        }

        // フライング
        if (ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1,InputManager::JOYPAD_BTN::DOWN))
        {
            StopSoundMem(bgm_);
            StopSoundMem(fallingSe_);
            StopSoundMem(noiseSe_);
            StopSoundMem(countSe_);
            StopSoundMem(errorSe_);
            StopSoundMem(triviaSe_);
            isFlash_ = true;
            pressFrame_ = -1;
            timer_ = 0;
            gameState_ = GameState::RESULT;
        }
        break;

    case GameState::GO:
        FakeFalling();
        // プレイヤー
        if ((ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN)) && !isPressed_)
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
                if (playerWin_ < 2 && cpuWin_ < 2)
                {
                    PlaySoundMem(lostSe_, DX_PLAYTYPE_BACK);
                }
                if (playerWin_ >= 2 || cpuWin_ >= 2)
                {
                    PlaySoundMem(overSe_, DX_PLAYTYPE_BACK);
                }
            }
            else if (!isPressed_)
            {
                cpuWin_++;
                if (playerWin_ < 2 && cpuWin_ < 2)
                {
                    PlaySoundMem(lostSe_, DX_PLAYTYPE_BACK);
                }
                if (playerWin_ >= 2 || cpuWin_ >= 2 || round_ >= 3)
                {
                    PlaySoundMem(overSe_, DX_PLAYTYPE_BACK);
                }
            }
            else if (!cpuPressed_ || pressFrame_ < cpuPressFrame_)
            {
                playerWin_++;
                if (playerWin_ < 2 && cpuWin_ < 2)
                {
                    PlaySoundMem(pointSe_, DX_PLAYTYPE_BACK);
                }
                if (playerWin_ >= 2 || cpuWin_ >= 2 || round_ >= 3)
                {
                    PlaySoundMem(clearSe_, DX_PLAYTYPE_BACK);
                }
            }
            else
            {
                cpuWin_++;
                if (playerWin_ < 2 && cpuWin_ < 2)
                {
                    PlaySoundMem(lostSe_, DX_PLAYTYPE_BACK);
                }
                if (playerWin_ >= 2 || cpuWin_ >= 2 || round_ >= 3)
                {
                    PlaySoundMem(overSe_, DX_PLAYTYPE_BACK);
                }
            }
        }

        // 少し待って次ラウンドへ
        if (timer_ > 120)
        {
            // 2勝したら終了
            if (playerWin_ >= 2 || cpuWin_ >= 2 || round_ >= 3)
            {
                if (timer_ > 240)
                {
                    // 完全終了
                    isReturn_ = true;
                }
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
    //DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);
    FakeWaveBackground();
    FakeTrivia();

    DrawRotaGraph(centerX + fallX_, centerY + fallY_, 0.6, angle_, backUIImg_, true);

    // カウント
    DrawCountUI();

    FakeSystemError();

    switch (gameState_)
    {
    case GameState::WAIT:
        DrawRotaGraph(centerX, centerY, 0.5, 0.0, readyUIImg_, true);
        break;

    case GameState::READY:
        DrawFadeReady();
        FakeNoise();
        FakeCountDown();
        break;

    case GameState::GO:
        DrawFadeReady();
        FakeNoise();
        FakeCountDown();
        DrawRotaGraph(centerX, centerY, 0.5, 0.0, pressUIImg_, true);
        break;

    case GameState::RESULT:
    {
        char buf[64];

        if (pressFrame_ < 0)
        {
            //DrawString(centerX / 2 - 80, screenY / 2, "FLYING!", red);
            //DrawRotaGraph(centerX, centerY, 0.4, 0.0, flyingUIImg_, true);
        }
        else if (isPressed_)
        {
            //DrawRotaGraph(centerX / 2, screenY / 2, 0.25, 0.0, playerTimeUIImg_, true);
            //sprintf_s(buf, "TIME: %d", pressFrame_);
            //DrawString(centerX / 2 - 80, screenY / 2 - 20, buf, green);
        }

        if (cpuPressed_)
        {
            //sprintf_s(buf, "TIME: %d", cpuPressFrame_);
            //DrawString(centerX + centerX / 2 - 80, screenY / 2, buf, red);
        }

        // 勝敗表示
        if (playerWin_ < 2 && cpuWin_ < 2)
        {
            if (pressFrame_ < 0)
            {
                DrawRotaGraph(centerX, centerY, 0.4, 0.0, flyingUIImg_, true);
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
        //DrawString(centerX + centerX / 2 - 30, screenY / 2 + 40, "PUSH!", red);
    }
}

void FirstPressGame::DrawUI(void)
{
}

void FirstPressGame::Reset(void)
{
    Init();
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
    fallSpeed_ = 0.0f;
    angle_ = 0.0f;
    fallX_ = 0;
    fallY_ = 0;
    isFall_ = false;
    countDownFrame_ = 0;
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
        DrawRotaGraph(150 + fallX_, 150 + fallY_, 0.2, angle_, countUIImg_, true);
        DrawRotaGraph(200 + fallX_, 150 + fallY_, 0.2, angle_, countUIImg_, true);
    }
    else if (playerWin_ == 1)
    {
        DrawRotaGraph(150 + fallX_, 150 + fallY_, 0.2, angle_, winCountUIImg_, true);
        DrawRotaGraph(200 + fallX_, 150 + fallY_, 0.2, angle_, countUIImg_, true);
    }
    else if (playerWin_ == 2)
    {
        DrawRotaGraph(150 + fallX_, 150 + fallY_, 0.2, angle_, winCountUIImg_, true);
        DrawRotaGraph(200 + fallX_, 150 + fallY_, 0.2, angle_, winCountUIImg_, true);
    }

    // CPU側
    if (cpuWin_ <= 0)
    {
        DrawRotaGraph(880 + fallX_, 150 + fallY_, 0.2, angle_, countUI2Img_, true);
        DrawRotaGraph(830 + fallX_, 150 + fallY_, 0.2, angle_, countUI2Img_, true);
    }
    else if (cpuWin_ == 1)
    {
        DrawRotaGraph(880 + fallX_, 150 + fallY_, 0.2, angle_, loseCountUIImg_, true);
        DrawRotaGraph(830 + fallX_, 150 + fallY_, 0.2, angle_, countUI2Img_, true);
    }
    else if (cpuWin_ == 2)
    {
        DrawRotaGraph(880 + fallX_, 150 + fallY_, 0.2, angle_, loseCountUIImg_, true);
        DrawRotaGraph(830 + fallX_, 150 + fallY_, 0.2, angle_, loseCountUIImg_, true);
    }
}

void FirstPressGame::Flash(void)
{
    fallSpeed_ = 0.0f;
    angle_ = 0.0f;
    fallX_ = 0;
    fallY_ = 0;
    isFall_ = false;
    
    int col = 0;

    if (pressFrame_ < 0)
    {
        col = GetColor(238, 180, 238);
    }
    else if (!isPressed_)
    {
        col = GetColor(238, 180, 238);
    }
    else if (!cpuPressed_ || pressFrame_ < cpuPressFrame_)
    {
        col = GetColor(0, 180, 255);
    }
    else
    {
        col = GetColor(238, 180, 238);
    }


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
            col,
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

void FirstPressGame::FakeFalling(void)
{
    if (!isFall_)
    {
        int rand = GetRand(900);
        if (rand == 0)
        {
            PlaySoundMem(fallingSe_, DX_PLAYTYPE_BACK);
            isFall_ = true;
        }
    }

    // 落下処理
    if (isFall_)
    {
        // 重力
        fallSpeed_ += 0.2f;

        // 落下
        fallY_ += (int)fallSpeed_;

        // 少し右へ流す
        fallX_ += 2;

        // 回転
        angle_ += 0.01f;
    }

}

void FirstPressGame::FakeNoise()
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;
    int rand = GetRand(400);

    if (rand == 0)
    {
        PlaySoundMem(noiseSe_, DX_PLAYTYPE_BACK);

        //-----------------------------------------
        // 白っぽいフラッシュ
        //-----------------------------------------
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 35);

        DrawBox(
            0,
            0,
            screenX,
            screenY,
            GetColor(180, 220, 255),
            TRUE
        );

        //-----------------------------------------
        // ビリッとした横グリッチ
        //-----------------------------------------
        SetDrawBlendMode(DX_BLENDMODE_ADD, 180);

        for (int i = 0; i < 7; i++)
        {
            int y = GetRand(screenY);

            // 横ズレ
            int offset = GetRand(80) - 40;

            DrawBox(
                offset,
                y,
                screenX + offset,
                y + GetRand(3) + 2,
                GetColor(200, 240, 255),
                TRUE
            );
        }

        //-----------------------------------------
        // 小さい発光ノイズ
        //-----------------------------------------
        for (int i = 0; i < 20; i++)
        {
            int x = GetRand(screenX);
            int y = GetRand(screenY);

            DrawCircle(
                x,
                y,
                GetRand(2) + 1,
                GetColor(255, 255, 255),
                TRUE
            );
        }

        //-----------------------------------------
        // 画面端に電気っぽい線
        //-----------------------------------------
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

        DrawLine(
            0,
            GetRand(screenY),
            screenX,
            GetRand(screenY),
            GetColor(120, 200, 255)
        );

        //-----------------------------------------
        // ブレンド解除
        //-----------------------------------------
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void FirstPressGame::FakeCountDown()
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    //-----------------------------------------
    // ランダム発動
    //-----------------------------------------
    static bool isCount = false;

    if (!isCount)
    {
        int rand = GetRand(700);

        if (rand == 0)
        {
            PlaySoundMem(countSe_, DX_PLAYTYPE_BACK);
            isCount = true;
            countDownFrame_ = 0;
        }
    }

    //-----------------------------------------
    // カウントしてないなら終了
    //-----------------------------------------
    if (!isCount)
    {
        return;
    }

    //-----------------------------------------
    // フレーム進行
    //-----------------------------------------
    countDownFrame_++;

    const int interval = 60;

    const char* text = "";

    //-----------------------------------------
    // 数字
    //-----------------------------------------
    if (countDownFrame_ < interval)
    {
        text = "3";
    }
    else if (countDownFrame_ < interval * 2)
    {
        text = "2";
    }
    else if (countDownFrame_ < interval * 3)
    {
        text = "1";
    }
    else
    {
        isCount = false;
        return;
    }

    //-----------------------------------------
    // 超大きめスケール
    //-----------------------------------------
    int localFrame = countDownFrame_ % interval;

    double scale =
        4.0 - (localFrame / (double)interval) * 1.0;

    //-----------------------------------------
    // フェード
    //-----------------------------------------
    int alpha = 255;

    if (localFrame < 6)
    {
        alpha = localFrame * 40;
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    //-----------------------------------------
    // サイズ取得
    //-----------------------------------------
    int textWidth = GetDrawStringWidth(text, strlen(text));

    //-----------------------------------------
    // 青グロー
    //-----------------------------------------
    DrawExtendString(
        centerX - (textWidth * scale) / 2,
        centerY - 120,
        scale + 0.2,
        scale + 0.2,
        text,
        GetColor(80, 180, 255)
    );

    //-----------------------------------------
    // 影
    //-----------------------------------------
    DrawExtendString(
        centerX - (textWidth * scale) / 2 + 8,
        centerY - 120 + 8,
        scale,
        scale,
        text,
        GetColor(0, 0, 0)
    );

    //-----------------------------------------
    // 本体
    //-----------------------------------------
    DrawExtendString(
        centerX - (textWidth * scale) / 2,
        centerY - 120,
        scale,
        scale,
        text,
        GetColor(255, 255, 255)
    );

    //-----------------------------------------
    // ブレンド解除
    //-----------------------------------------
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void FirstPressGame::FakeWaveBackground()
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    const float scale = 0.7f;

    //-----------------------------------------
    // 状態管理
    //-----------------------------------------
    static bool isWave = false;
    static int waveTimer = 0;
    static float wavePower = 0.0f;

    //-----------------------------------------
    // READYとGO以外
    //-----------------------------------------
    if (!(gameState_ == GameState::READY ||
        gameState_ == GameState::GO))
    {
        // 状態リセット
        isWave = false;
        waveTimer = 0;
        wavePower = 0.0f;

        DrawRotaGraph(centerX, centerY, scale, 0.0, backImg_, TRUE);
        return;
    }

    //-----------------------------------------
    // ランダム発生
    //-----------------------------------------
    if (!isWave)
    {
        if (GetRand(1200) == 0)
        {
            isWave = true;
            waveTimer = 0;
        }
    }

    //-----------------------------------------
    // 波処理
    //-----------------------------------------
    if (isWave)
    {
        waveTimer++;

        // 強くなる
        if (waveTimer < 30)
        {
            wavePower += 1.0f;
        }

        // 徐々に戻る
        if (waveTimer > 240)
        {
            wavePower -= 0.4f;
        }

        // 終了
        if (wavePower <= 0.0f && waveTimer > 240)
        {
            wavePower = 0.0f;
            isWave = false;
        }
    }

    //-----------------------------------------
    // 最大値
    //-----------------------------------------
    if (wavePower > 25.0f)
    {
        wavePower = 25.0f;
    }

    //-----------------------------------------
    // 画像サイズ
    //-----------------------------------------
    int imgW, imgH;
    GetGraphSize(backImg_, &imgW, &imgH);

    //-----------------------------------------
    // 描画サイズ
    //-----------------------------------------
    int drawW = (int)(imgW * scale);
    int drawH = (int)(imgH * scale);

    //-----------------------------------------
    // 左上座標
    //-----------------------------------------
    int drawX = centerX - drawW / 2;
    int drawY = centerY - drawH / 2;

    //-----------------------------------------
    // 波設定
    //-----------------------------------------
    const int sliceHeight = 4;

    float time = GetNowCount() * 0.005f;

    //-----------------------------------------
    // 波がない時は完全通常描画
    //-----------------------------------------
    if (wavePower <= 0.0f)
    {
        DrawRotaGraph(centerX, centerY, scale, 0.0, backImg_, TRUE);
        return;
    }

    //-----------------------------------------
    // 波描画
    //-----------------------------------------
    for (int y = 0; y < drawH; y += sliceHeight)
    {
        //-------------------------------------
        // 横揺れ
        //-------------------------------------
        int waveX =
            (int)(sin(y * 0.02f + time) * wavePower);

        //-------------------------------------
        // 元画像座標
        //-------------------------------------
        float srcTopF =
            (float)y / drawH * imgH;

        float srcBottomF =
            (float)(y + sliceHeight) / drawH * imgH;

        int srcTop = (int)srcTopF;
        int srcHeight = (int)(srcBottomF - srcTopF);

        //-------------------------------------
        // 描画
        //-------------------------------------
        DrawRectExtendGraph(
            drawX + waveX,
            drawY + y,

            drawX + drawW + waveX,
            drawY + y + sliceHeight,

            0,
            srcTop,

            imgW,
            srcHeight,

            backImg_,
            TRUE
        );
    }
}

void FirstPressGame::FakeSystemError(void)
{
    //-----------------------------------------
    // 状態
    //-----------------------------------------
    static bool isShow = false;
    static int timer = 0;
    static int prevWindowCount = 0;

    //-----------------------------------------
    // READY / GO 以外ならリセット
    //-----------------------------------------
    if (!(gameState_ == GameState::READY ||
        gameState_ == GameState::GO))
    {
        isShow = false;
        timer = 0;
        prevWindowCount = 0;
        return;
    }

    //-----------------------------------------
    // ランダム発動
    //-----------------------------------------
    if (!isShow)
    {
        if (GetRand(1800) == 0)
        {
            isShow = true;
            timer = 0;
            prevWindowCount = 0;
        }
    }

    //-----------------------------------------
    // 発動してない
    //-----------------------------------------
    if (!isShow)
    {
        return;
    }

    //-----------------------------------------
    // 時間
    //-----------------------------------------
    timer++;

    //-----------------------------------------
    // エラー文
    //-----------------------------------------
    const char* errorList[] =
    {
        "SIGNAL ERROR",
        "SYNC FAILED",
        "UNKNOWN SIGNAL",
        "SYSTEM OVERLOAD",
        "MEMORY ERROR",
        "CPU FAILURE"
    };

    //-----------------------------------------
    // 徐々に増殖
    //-----------------------------------------
    int windowCount = timer / 20;

    if (windowCount > 5)
    {
        windowCount = 5;
    }

    //-----------------------------------------
    // ウィンドウが増えた瞬間だけSE
    //-----------------------------------------
    if (windowCount > prevWindowCount)
    {
        PlaySoundMem(errorSe_, DX_PLAYTYPE_BACK);
    }

    prevWindowCount = windowCount;

    //-----------------------------------------
    // 描画
    //-----------------------------------------
    for (int i = 0; i < windowCount; i++)
    {
        int x = 20 + i * 28;
        int y = 20 + i * 20;

        x += GetRand(2) - 1;
        y += GetRand(2) - 1;

        int alpha = 220;

        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            alpha
        );

        //-------------------------------------
        // 背景
        //-------------------------------------
        DrawBox(
            x,
            y,
            x + 380,
            y + 140,
            GetColor(20, 35, 120),
            TRUE
        );

        //-------------------------------------
        // タイトルバー
        //-------------------------------------
        DrawBox(
            x,
            y,
            x + 380,
            y + 24,
            GetColor(40, 100, 255),
            TRUE
        );

        //-------------------------------------
        // 枠
        //-------------------------------------
        DrawBox(
            x,
            y,
            x + 380,
            y + 140,
            GetColor(120, 180, 255),
            FALSE
        );

        //-------------------------------------
        // タイトル
        //-------------------------------------
        DrawString(
            x + 10,
            y + 5,
            "SYSTEM WARNING",
            GetColor(255, 255, 255)
        );

        //-------------------------------------
        // エラー
        //-------------------------------------
        DrawString(
            x + 16,
            y + 45,
            errorList[(timer / 30 + i) % 6],
            GetColor(180, 220, 255)
        );

        //-------------------------------------
        // コード
        //-------------------------------------
        char buf[64];

        sprintf_s(
            buf,
            "CODE : 0x%04X",
            1000 + i * 731
        );

        DrawString(
            x + 16,
            y + 70,
            buf,
            GetColor(255, 120, 120)
        );

        //-------------------------------------
        // 状態
        //-------------------------------------
        DrawString(
            x + 16,
            y + 95,
            "STATUS : UNSTABLE",
            GetColor(255, 255, 255)
        );

        //-------------------------------------
        // ノイズ線
        //-------------------------------------
        for (int n = 0; n < 2; n++)
        {
            int lineY =
                y + 40 + GetRand(80);

            DrawLine(
                x + 8,
                lineY,
                x + 372,
                lineY,
                GetColor(120, 180, 255)
            );
        }
    }

    //-----------------------------------------
    // ブレンド解除
    //-----------------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

void FirstPressGame::FakeTrivia(void)
{
    //-----------------------------------------
    // READY / GO 以外ならリセット
    //-----------------------------------------
    static int triviaCount = 0;
    static int timer = 0;
    static int triviaIndex[20];

    if (!(gameState_ == GameState::READY ||
        gameState_ == GameState::GO))
    {
        triviaCount = 0;
        timer = 0;
        return;
    }

    //-----------------------------------------
    // うんちく
    //-----------------------------------------
    const char* triviaList[] =
    {
        "人は集中している時ほど瞬きが減少します",
        "人間は視覚より音への反応が速いです",
        "ストレス下では反応速度が低下します",
        "カフェインは約30分後に効果が最大になります",
        "脳は反応する前に未来を予測しています",
        "タコの心臓は3つあります",
        "深海魚の多くは発光能力を持っています",
        "人は無意識に画面端の文字を読んでしまいます",
        "集中力は一度乱れると戻るまで時間がかかります",
        "人間の脳は突然の光に強く反応します",

        "シャチは睡眠中でも脳の半分だけ起きています",
        "人間の反応速度は加齢で徐々に低下します",
        "青色光は覚醒状態を強める効果があります",
        "人は静かな環境ほど小さな音に敏感になります",
        "猫は人間より高周波の音を聞き取れます",
        "集中状態では周囲の情報が見えにくくなります",
        "緊張すると視野が狭くなる現象があります",
        "人間の脳は予想外の動きに強く反応します",
        "突然の点滅は集中力を乱しやすいです",
        "長時間の集中は判断能力を低下させます",

        "ペンギンは水中では非常に高速で泳ぎます",
        "脳は見えていない部分を自動で補完しています",
        "視線は無意識に動くものを追いかけます",
        "反応速度には個人差があります",
        "人は読めそうな文章をつい読んでしまいます",
        "疲労は判断速度に大きく影響します",
        "短期記憶は数秒で消えることがあります",
        "人間は赤色を見ると警戒しやすくなります",
        "集中中は時間感覚が変化しやすいです",
        "人は予測できない音に驚きやすいです",

        "イルカは鏡で自分を認識できます",
        "視界の端の動きにも脳は反応しています",
        "脳は常に危険を予測しています",
        "人は選択肢が多いほど迷いやすくなります",
        "一瞬の迷いでも反応は遅れます",
        "睡眠不足は反応速度を低下させます",
        "人間の神経伝達は電気信号で行われています",
        "光の点滅は集中を妨害しやすいです",
        "人は突然のノイズに意識を奪われます",
        "集中時は呼吸が浅くなることがあります",

        "フクロウは首を大きく回転できます",
        "脳は無意識に周囲を監視しています",
        "人間は危険を感じると反射的に反応します",
        "指先は非常に敏感な感覚器官です",
        "緊張状態では心拍数が上昇します",
        "反応には脳と筋肉の伝達時間があります",
        "突然の表示変化は視線を引きつけます",
        "人は移動する文字を追いやすいです",
        "視覚情報は脳で処理されてから認識されます",
        "人間は完全に集中し続けることができません",

        "クラゲには脳が存在しません",
        "一部の深海生物は自ら発光します",
        "人間は不規則な動きに注意を奪われます",
        "脳は静寂より変化に反応します",
        "瞬間的な判断には経験が影響します",
        "人は予想外の情報に視線を向けます",
        "視界端の光でも脳は検知しています",
        "長時間の緊張は疲労を加速させます",
        "反応速度は精神状態でも変化します",
        "人間は読めない文字でも形を認識します"
    };

    const int maxTrivia =
        sizeof(triviaList) / sizeof(triviaList[0]);

    //-----------------------------------------
    // 時間
    //-----------------------------------------
    timer++;

    //-----------------------------------------
    // ランダム追加
    //-----------------------------------------
    if (GetRand(200) == 0)
    {
        PlaySoundMem(triviaSe_, DX_PLAYTYPE_BACK);

        if (triviaCount < 20)
        {
            triviaIndex[triviaCount] =
                GetRand(maxTrivia - 1);

            triviaCount++;
        }
    }

    //-----------------------------------------
    // 表示位置
    //-----------------------------------------
    int startX = 40;
    int startY = 620;

    //-----------------------------------------
    // 最大表示数
    //-----------------------------------------
    int visibleCount = triviaCount;

    if (visibleCount > 6)
    {
        visibleCount = 6;
    }

    //-----------------------------------------
    // 描画
    //-----------------------------------------
    for (int i = 0; i < visibleCount; i++)
    {
        //-------------------------------------
        // 古いものから表示
        //-------------------------------------
        int index =
            triviaCount - visibleCount + i;

        //-------------------------------------
        // 位置
        //-------------------------------------
        int x = startX;
        int y = startY - (visibleCount - 1 - i) * 42;

        //-------------------------------------
        // 透明度
        //-------------------------------------
        int alpha = 120 + i * 20;

        if (alpha > 220)
        {
            alpha = 220;
        }

        //-------------------------------------
        // ブレンド
        //-------------------------------------
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            alpha
        );

        //-------------------------------------
        // 背景
        //-------------------------------------
        DrawBox(
            x - 10,
            y - 6,
            x + 760,
            y + 24,
            GetColor(10, 20, 40),
            TRUE
        );

        //-------------------------------------
        // 枠
        //-------------------------------------
        DrawBox(
            x - 10,
            y - 6,
            x + 760,
            y + 24,
            GetColor(100, 180, 255),
            FALSE
        );

        //-------------------------------------
        // タイトル
        //-------------------------------------
        DrawString(
            x,
            y,
            "[ INFO ]",
            GetColor(120, 200, 255)
        );

        //-------------------------------------
        // 本文
        //-------------------------------------
        DrawString(
            x + 100,
            y,
            triviaList[triviaIndex[index]],
            GetColor(255, 255, 255)
        );
    }

    //-----------------------------------------
    // ブレンド解除
    //-----------------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}