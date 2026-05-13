#pragma once
#include "../ActorBase.h"
class Grid :
    public ActorBase
{
public:

	static constexpr float GRID_SIZE = 50.0f; // セルのサイズ
    static constexpr VECTOR DEFAULT_SCALE = { 0.1f,0.1f,0.1f };


    Grid(void);
    virtual ~Grid(void);

    virtual void Init(void) override;
    virtual void Update(void) override;
    virtual void Draw(void) override;

	void SetBoardPosition(int x, int y);

private:

	int x_, y_; // 盤面座標

	void UpdateTransform(void);
};

