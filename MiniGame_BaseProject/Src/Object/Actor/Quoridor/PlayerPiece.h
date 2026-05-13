#pragma once
#include "../ActorBase.h"

class PlayerPiece :
    public ActorBase
{
public:

	static constexpr float GRID_SIZE = 50.0f; // セルのサイズ

    PlayerPiece(void);
    virtual ~PlayerPiece(void);
    virtual void Init(void) override;
    virtual void Update(void) override;
    virtual void Draw(void) override;

	void SetBoardPosition(int x, int y);
	void SetColor(float r, float g, float b);

private:

    int x_, y_; // 盤面座標

	void UpdateTransform(void);

};
