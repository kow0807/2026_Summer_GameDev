#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/Setting.h"
#include "../../Application.h"
#include "QuizGame.h"

#include <algorithm>
#include <random>
#include <ctime>

int GetCenterX(const char* text, int fontHandle)
{
    int width = GetDrawStringWidthToHandle(
        text,
        static_cast<int>(strlen(text)),
        fontHandle);

    return (Application::SCREEN_SIZE_X / 2) - (width / 2);
}

QuizGame::QuizGame(void)
{
}

QuizGame::~QuizGame(void)
{
}

void QuizGame::Init(void)
{
    backImg_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_BACK).handleId_;

    bgm_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_BGM).handleId_;
    readySe_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_READY_SE).handleId_;
    correctSe_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_CORRECT).handleId_;
    buzzerSe_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_BUZZER).handleId_;
    cursorSe_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_CURSOR).handleId_;
    resultSe_ = resMng_.Load(ResourceManager::SRC::QUIZ_GAME_RESULT).handleId_;

    titleFont_ = CreateFontToHandle(
        "游明朝",
        52,
        5);

    uiFont_ = CreateFontToHandle(
        "游明朝",
        28,
        3);

    gameState_ = GameState::READY;

    currentQuizIndex_ = 0;
    score_ = 0;

    selectIndex_ = 0;

    countdownFrame_ = 0;
    countdownNumber_ = 3;

    answerEffectFrame_ = 0;
    isCorrectAnswer_ = false;

    // 15秒
    maxTimeFrame_ = 60 * 15;
    timeLimitFrame_ = maxTimeFrame_;

    quizList_.clear();

    std::vector<QuizData> allQuiz;

    // =====================================
    // クイズ30問
    // =====================================

    allQuiz.push_back({
        "日本の首都は？",
        { "大阪", "東京", "福岡", "北海道" },
        1
        });

    allQuiz.push_back({
        "日本で最も多い血液型は？",
        { "A型", "B型", "O型", "AB型" },
        0
        });

    allQuiz.push_back({
        "世界で最も売れているゲーム機は？",
        { "Wii", "PS2", "Switch", "3DS" },
        1
        });

    allQuiz.push_back({
        "富士山の高さは？",
        { "3776m", "2999m", "5000m", "1234m" },
        0
        });

    allQuiz.push_back({
        "地球は太陽から数えて何番目？",
        { "1番目", "2番目", "3番目", "4番目" },
        2
        });

    allQuiz.push_back({
        "サメは魚類？",
        { "魚類", "哺乳類", "両生類", "昆虫" },
        0
        });

    allQuiz.push_back({
        "日本で一番長い川は？",
        { "利根川", "信濃川", "筑後川", "四万十川" },
        1
        });

    allQuiz.push_back({
        "マリオの職業として正式なのは？",
        { "配管工", "シェフ", "医者", "大工" },
        0
        });

    allQuiz.push_back({
        "オーロラが発生する主な原因は？",
        { "雲", "月光", "太陽風", "雷" },
        2
        });

    allQuiz.push_back({
        "人間の骨の数は約何本？",
        { "106本", "206本", "306本", "406本" },
        1
        });

    allQuiz.push_back({
        "日本の通貨は？",
        { "ドル", "ウォン", "円", "ユーロ" },
        2
        });

    allQuiz.push_back({
        "バナナはどの仲間？",
        { "木", "草", "コケ", "野菜" },
        1
        });

    allQuiz.push_back({
        "鉛筆の芯の主成分は？",
        { "鉛", "炭", "黒鉛", "鉄" },
        2
        });

    allQuiz.push_back({
        "フラミンゴがピンク色な理由は？",
        { "生まれつき", "日焼け", "血の色", "食べ物" },
        3
        });

    allQuiz.push_back({
        "1000mは何km？",
        { "1km", "10km", "100km", "0.1km" },
        0
        });

    allQuiz.push_back({
        "オリンピックは何年ごと？",
        { "2年", "3年", "4年", "5年" },
        2
        });

    allQuiz.push_back({
        "日本で最も広い湖は？",
        { "霞ヶ浦", "十和田湖", "浜名湖", "琵琶湖" },
        3
        });

    allQuiz.push_back({
        "世界で一番大きい動物は？",
        { "ゾウ", "クジラ", "キリン", "サメ" },
        1
        });

    allQuiz.push_back({
        "日本で最初のカップラーメンは？",
        { "UFO", "どん兵衛", "チキンラーメン", "カップヌードル" },
        3
        });

    allQuiz.push_back({
        "日本の国鳥は？",
        { "ツル", "キジ", "スズメ", "ハト" },
        1
        });

    allQuiz.push_back({
        "タピオカの原料は？",
        { "米", "小麦", "豆", "芋" },
        3
        });

    allQuiz.push_back({
        "人間の血管を全部つなぐと？",
        { "約1km", "約10km", "約10万km", "約100万km" },
        2
        });

    allQuiz.push_back({
        "世界で最も多い鳥は？",
        { "ニワトリ", "ハト", "スズメ", "カラス" },
        0
        });

    allQuiz.push_back({
        "地球の衛星は？",
        { "火星", "月", "太陽", "金星" },
        1
        });

    allQuiz.push_back({
        "日本の1円玉の材料は？",
        { "鉄", "銅", "アルミ", "銀" },
        2
        });

    allQuiz.push_back({
        "エッフェル塔がある都市は？",
        { "ローマ", "ロンドン", "ニューヨーク", "パリ" },
        3
        });

    allQuiz.push_back({
        "1年は約何日？",
        { "265日", "365日", "465日", "565日" },
        1
        });

    allQuiz.push_back({
        "日本で一番高い山は？",
        { "阿蘇山", "高尾山", "富士山", "桜島" },
        2
        });

    allQuiz.push_back({
        "タコの足は何本？",
        { "6本", "8本", "10本", "12本" },
        1
        });

    allQuiz.push_back({
        "日本の国民の祝日で一番新しいものは？",
        { "山の日", "海の日", "体育の日", "文化の日" },
        0
        });

    // =====================================
    // ランダムシャッフル
    // =====================================

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(
        allQuiz.begin(),
        allQuiz.end(),
        g);

    // =====================================
    // 10問だけ選出
    // =====================================

    for (int i = 0; i < 10; i++)
    {
        quizList_.push_back(allQuiz[i]);
    }

    resultFrame_ = 0;
    isReturn_ = false;
}

void QuizGame::Update(void)
{
    switch (gameState_)
    {
    case GameState::READY:
        UpdateReady();
        break;

    case GameState::PLAY:
        UpdatePlay();
        break;

    case GameState::RESULT:
        UpdateResult();
        break;
    }
}

void QuizGame::UpdateReady(void)
{
    if (countdownNumber_ == 3 && countdownFrame_ == 0)
    {
        PlaySoundMem(readySe_, DX_PLAYTYPE_BACK);
    }

    countdownFrame_++;

    if (countdownFrame_ >= 60)
    {
        countdownFrame_ = 0;
        countdownNumber_--;

        if (countdownNumber_ <= 0)
        {
            PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
            gameState_ = GameState::PLAY;
        }
    }
}

void QuizGame::UpdatePlay(void)
{
    InputManager& ins = InputManager::GetInstance();

    if (answerEffectFrame_ > 0)
    {
        answerEffectFrame_--;

        if (answerEffectFrame_ <= 0)
        {
            currentQuizIndex_++;

            selectIndex_ = 0;

            timeLimitFrame_ = maxTimeFrame_;

            if (currentQuizIndex_ >= quizList_.size())
            {
                PlaySoundMem(resultSe_, DX_PLAYTYPE_BACK);
                StopSoundMem(bgm_);
                gameState_ = GameState::RESULT;
            }
        }

        return;
    }

    timeLimitFrame_--;

    if (timeLimitFrame_ <= 0)
    {
        isCorrectAnswer_ = false;
        answerEffectFrame_ = 120;

        return;
    }

    if (ins.IsTrgDown(KEY_INPUT_UP) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP))
    {
        PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK);
        selectIndex_--;

        if (selectIndex_ < 0)
        {
            selectIndex_ = 3;
        }
    }

    if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))
    {
        PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK);
        selectIndex_++;

        if (selectIndex_ > 3)
        {
            selectIndex_ = 0;
        }
    }

    if (ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
    {
        if (selectIndex_ == quizList_[currentQuizIndex_].answerIndex)
        {
            PlaySoundMem(correctSe_, DX_PLAYTYPE_BACK);
            score_++;
            isCorrectAnswer_ = true;
        }
        else
        {
            PlaySoundMem(buzzerSe_, DX_PLAYTYPE_BACK);
            isCorrectAnswer_ = false;
        }

        answerEffectFrame_ = 120;
    }
}

void QuizGame::UpdateResult(void)
{
    InputManager& ins = InputManager::GetInstance();

    resultFrame_++;

    // 6.5秒後(60fps想定)
    if (resultFrame_ >= 390)
    {
        isReturn_ = true;
    }
}

void QuizGame::Draw(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);

    DrawBox(
        0,
        0,
        screenX,
        screenY,
        GetColor(0, 0, 20),
        true);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    switch (gameState_)
    {
    case GameState::READY:
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

        DrawBox(
            centerX - 230,
            centerY - 150,
            centerX + 230,
            centerY + 150,
            GetColor(0, 180, 255),
            true);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        DrawBox(
            centerX - 210,
            centerY - 130,
            centerX + 210,
            centerY + 130,
            GetColor(10, 20, 40),
            true);

        DrawBox(
            centerX - 210,
            centerY - 130,
            centerX + 210,
            centerY + 130,
            GetColor(0, 220, 255),
            false);

        DrawStringToHandle(
            GetCenterX("READY", titleFont_),
            centerY - 80,
            "READY",
            GetColor(255, 255, 255),
            titleFont_);

        char countText[8];

        sprintf_s(
            countText,
            "%d",
            countdownNumber_);

        DrawStringToHandle(
            GetCenterX(countText, titleFont_),
            centerY + 10,
            countText,
            GetColor(0, 255, 255),
            titleFont_);

        break;
    }

    case GameState::PLAY:

        DrawQuiz();

        break;

    case GameState::RESULT:
    {
        bool isClear = (score_ >= 8);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

        DrawBox(
            centerX - 320,
            centerY - 220,
            centerX + 320,
            centerY + 220,
            isClear ?
            GetColor(0, 180, 255) :
            GetColor(255, 60, 60),
            true);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        DrawBox(
            centerX - 300,
            centerY - 200,
            centerX + 300,
            centerY + 200,
            GetColor(10, 20, 40),
            true);

        DrawBox(
            centerX - 300,
            centerY - 200,
            centerX + 300,
            centerY + 200,
            isClear ?
            GetColor(0, 220, 255) :
            GetColor(255, 80, 80),
            false);

        // =========================================
        // CLEAR / FAILED
        // =========================================

        if (isClear)
        {
            DrawStringToHandle(
                GetCenterX("CLEAR!!", titleFont_),
                centerY - 130,
                "CLEAR!!",
                GetColor(0, 255, 180),
                titleFont_);
        }
        else
        {
            DrawStringToHandle(
                GetCenterX("FAILED", titleFont_),
                centerY - 130,
                "FAILED",
                GetColor(255, 80, 80),
                titleFont_);
        }

        char scoreText[64];

        sprintf_s(
            scoreText,
            "正解数 %d / %d",
            score_,
            static_cast<int>(quizList_.size()));

        DrawStringToHandle(
            GetCenterX(scoreText, uiFont_),
            centerY - 20,
            scoreText,
            GetColor(255, 255, 255),
            uiFont_);

        // 判定メッセージ
        if (isClear)
        {
            DrawStringToHandle(
                GetCenterX("8問以上正解でクリア！", uiFont_),
                centerY + 100,
                "8問以上正解でクリア！",
                GetColor(0, 255, 180),
                uiFont_);
        }
        else
        {
            DrawStringToHandle(
                GetCenterX("8問以上正解でクリア！", uiFont_),
                centerY + 100,
                "8問以上正解でクリア！",
                GetColor(255, 120, 120),
                uiFont_);
        }
        break;
    }
    }
}

void QuizGame::DrawQuiz(void)
{
    int screenX = Application::SCREEN_SIZE_X;
    int screenY = Application::SCREEN_SIZE_Y;

    int centerX = screenX / 2;
    int centerY = screenY / 2;

    QuizData& quiz = quizList_[currentQuizIndex_];

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

    DrawBox(
        30,
        30,
        screenX - 30,
        screenY - 15,
        GetColor(0, 0, 20),
        true);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawBox(
        30,
        30,
        screenX - 30,
        screenY - 15,
        GetColor(0, 180, 255),
        false);

    DrawBox(
        50,
        40,
        screenX - 50,
        120,
        GetColor(15, 25, 45),
        true);

    DrawBox(
        50,
        40,
        screenX - 50,
        120,
        GetColor(0, 180, 255),
        false);

    DrawFormatStringToHandle(
        80,
        55,
        GetColor(0, 220, 255),
        uiFont_,
        "QUESTION %d / 10",
        currentQuizIndex_ + 1);

    int timeSec = timeLimitFrame_ / 60;

    DrawStringToHandle(
        screenX - 260,
        50,
        "TIME",
        GetColor(255, 255, 255),
        uiFont_);

    DrawBox(
        screenX - 260,
        85,
        screenX - 60,
        105,
        GetColor(30, 30, 30),
        true);

    float rate =
        (float)timeLimitFrame_ /
        (float)maxTimeFrame_;

    int barWidth = (int)(200 * rate);

    DrawBox(
        screenX - 260,
        85,
        screenX - 260 + barWidth,
        105,
        rate > 0.3f ?
        GetColor(0, 255, 180) :
        GetColor(255, 80, 80),
        true);

    DrawBox(
        screenX - 260,
        85,
        screenX - 60,
        105,
        GetColor(255, 255, 255),
        false);

    DrawFormatStringToHandle(
        screenX - 110,
        52,
        GetColor(255, 255, 255),
        uiFont_,
        "%02d",
        timeSec);

    DrawBox(
        80,
        160,
        screenX - 80,
        270,
        GetColor(20, 20, 35),
        true);

    DrawBox(
        80,
        160,
        screenX - 80,
        270,
        GetColor(0, 180, 255),
        false);

    DrawStringToHandle(
        120,
        205,
        quiz.question.c_str(),
        GetColor(255, 255, 255),
        uiFont_);

    // =========================================
    // 選択肢表示
    // =========================================

    for (int i = 0; i < 4; i++)
    {
        int y = 305 + i * 75;

        bool isSelect = (i == selectIndex_);

        // 不正解時に正解を表示
        bool isCorrectChoice = false;

        if (answerEffectFrame_ > 0 && !isCorrectAnswer_)
        {
            isCorrectChoice =
                (i == quiz.answerIndex);
        }

        // 選択中の発光
        if (isSelect)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

            DrawBox(
                100,
                y - 4,
                screenX - 100,
                y + 60,
                GetColor(0, 180, 255),
                true);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        int backColor = GetColor(25, 25, 35);
        int frameColor = GetColor(80, 80, 120);

        // 通常選択
        if (isSelect)
        {
            backColor = GetColor(25, 60, 100);
            frameColor = GetColor(0, 220, 255);
        }

        // 正解表示
        if (isCorrectChoice)
        {
            backColor = GetColor(20, 100, 40);
            frameColor = GetColor(0, 255, 120);
        }

        DrawBox(
            110,
            y,
            screenX - 110,
            y + 55,
            backColor,
            true);

        DrawBox(
            110,
            y,
            screenX - 110,
            y + 55,
            frameColor,
            false);

        int numberColor =
            isSelect ?
            GetColor(0, 255, 255) :
            GetColor(180, 180, 180);

        if (isCorrectChoice)
        {
            numberColor = GetColor(0, 255, 120);
        }

        DrawFormatStringToHandle(
            145,
            y + 15,
            numberColor,
            uiFont_,
            "%d",
            i + 1);

        DrawFormatStringToHandle(
            220,
            y + 15,
            isCorrectChoice ?
            GetColor(0, 255, 120) :
            GetColor(255, 255, 255),
            uiFont_,
            "%s",
            quiz.choices[i].c_str());
    }

    // =========================================
    // 正解・不正解演出
    // =========================================

    if (answerEffectFrame_ > 0)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);

        DrawBox(
            0,
            0,
            screenX,
            screenY,
            isCorrectAnswer_ ?
            GetColor(0, 80, 40) :
            GetColor(80, 0, 0),
            true);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        if (isCorrectAnswer_)
        {
            DrawStringToHandle(
                GetCenterX("CORRECT!!", titleFont_),
                centerY - 30,
                "CORRECT!!",
                GetColor(0, 255, 120),
                titleFont_);
        }
        else
        {
            DrawStringToHandle(
                GetCenterX("WRONG...", titleFont_),
                centerY - 30,
                "WRONG...",
                GetColor(255, 80, 80),
                titleFont_);
        }
    }
}

void QuizGame::DrawUI(void)
{
}

void QuizGame::Reset(void)
{
    Init();
}