#pragma once
#include "GameBase.h"
#include <string>
#include <vector>

class QuizGame : public GameBase
{
public:

    enum class GameState
    {
        READY,
        PLAY,
        RESULT
    };

    struct QuizData
    {
        std::string question;
        std::string choices[4];
        int answerIndex;
    };

    QuizGame(void);
    ~QuizGame(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void DrawUI(void) override;
    void Reset(void) override;

private:

    GameState gameState_;

    int backImg_;

    std::vector<QuizData> quizList_;

    int bgm_;
    int readySe_;
    int correctSe_;
    int buzzerSe_;
    int cursorSe_;
    int resultSe_;

    int menuSe_;
    int cancelSe_;
    int moveSe_;
    int decideSEH_;

    bool isPause_;
    int pauseScreenHandle_;
    float pauseX_;
    int pauseSelect_;

    int explanationFontHandle_;

    int currentQuizIndex_;
    int score_;

    int selectIndex_;

    // READY
    int countdownFrame_;
    int countdownNumber_;

    // ê≥âââèo
    int answerEffectFrame_;
    bool isCorrectAnswer_;

    // êßå¿éûä‘
    int timeLimitFrame_;
    int maxTimeFrame_;

    int titleFont_;
    int uiFont_;

    int resultFrame_;

    void UpdateReady(void);
    void UpdatePlay(void);
    void UpdateResult(void);

    void DrawQuiz(void);

    bool PauseUpdate(void);
    void PauseDraw(void);
};