#pragma once
#include "../ActorBase.h"

class MiniShogiGrid :
    public ActorBase
{
public:

    MiniShogiGrid(void);
    ~MiniShogiGrid(void);

    void Init(void)override;
    void Update(void)override;
    void Draw(void)override;

    void SetBoardOffset(VECTOR offset);
    void SetCellSize(float size);

private:

    VECTOR boardOffset_;
    float cellSize_;


    VECTOR GetCellCenter(int x, int y) const;


};

