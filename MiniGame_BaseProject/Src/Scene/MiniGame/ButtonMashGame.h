#pragma once
#include "GameBase.h"

class ButtonMashGame : public GameBase
{
public:

    enum class GameState
    {
        READY,
        GO,
        RESULT
    };

    ButtonMashGame(void);
    ~ButtonMashGame(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void DrawUI(void) override;
    void Reset(void) override;

private:

    GameState gameState_;

    int backImg_;
    int backUIImg_;
    int readyUIImg_;
    int pressUIImg_;

    int winUIImg_;
    int loseUIImg_;

    bool isReady_;
    int readySe_;
    bool isBgm_;
    int bgm_;
    int pushSe_;
    int clearSe_;
    int overSe_;
    int pointSe_;
    int lostSe_;

    int menuSe_;
    int cancelSe_;
    int moveSe_;
    int decideSEH_;

    bool isPause_;
    int pauseScreenHandle_;
    float pauseX_;
    int pauseSelect_;

    int explanationFontHandle_;

    float battleRate_;

    int playerWinCount_;
    int cpuWinCount_;
    int countUIImg_;
    int winCountUIImg_;
    int countUI2Img_;
    int loseCountUIImg_;
    int pointUIImg_;
    int lostUIImg_;

    bool isPlayerWin_;

    int readyTimer_;
    int resultTimer_;

    float playerFlashPower_;
    float cpuFlashPower_;

    void DrawBattleLine(int borderX);
    void DrawBattleArea(int borderX);
    int GetBattleOffset(int y);
    void DrawCountUI(void);

    bool PauseUpdate(void);
    void PauseDraw(void);
};