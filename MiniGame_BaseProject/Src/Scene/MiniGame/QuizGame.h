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

public:

    QuizGame(void);
    ~QuizGame(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void DrawUI(void) override;
    void Reset(void) override;

private:

    void UpdateReady(void);
    void UpdatePlay(void);
    void UpdateResult(void);

    void DrawQuiz(void);

private:

    GameState gameState_;

    int backImg_;

    std::vector<QuizData> quizList_;

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
};