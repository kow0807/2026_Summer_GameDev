#pragma once
#include <memory>
#include "GameBase.h"

class Shogiban;
class Komadai;

class Cursor;
class Selector;
class MiniShogiBoard;
class MiniShogiRule;

class MiniShogi :
    public GameBase
{
public:

    MiniShogi(void);
    ~MiniShogi(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void DrawUI(void) override;
    void Reset(void) override;

private:


    std::unique_ptr<Shogiban> shogiban_;
    std::unique_ptr<Komadai> komadai_;

    std::unique_ptr<Cursor> cursor_;
    std::unique_ptr<Selector> selector_;
    std::unique_ptr<MiniShogiBoard> board_;
    std::unique_ptr<MiniShogiRule> rule_;

    bool isPlayerTurn_;

    void InputUpdate(void);

    void SelectUpdate(void);

};

