#pragma once
#include <DxLib.h>
#include "../ActorBase.h"

class Shogiban;
class Cursor;
class Selector;

class MiniShogiActor :
    public ActorBase
{
public:

    MiniShogiActor(
        Shogiban* board,
        Cursor* cursor,
        Selector* selector
    );

    virtual ~MiniShogiActor(void);

    virtual void Init(void) override;
    virtual void Update(void) override;
    virtual void Draw(void) override;

private:

    void DrawBoard(void);

    void DrawPieces(void);

    void DrawMoveList(void);

    VECTOR GetBoardPostion(int x, int y) const;

    Shogiban* board_;
    Cursor* cursor_;
    Selector* selector_;
};

