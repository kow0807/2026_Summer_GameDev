#pragma once
#include "../ActorBase.h"

class PlayerPiece :
    public ActorBase
{
public:

    static constexpr VECTOR DEFAULT_SCALE = { 0.1f, 0.1f, 0.1f };

    PlayerPiece(void);
    virtual ~PlayerPiece(void);
    virtual void Init(void) override;
    virtual void Update(void) override;
    virtual void Draw(void) override;

	void SetBoardPosition(int x, int y);
	void SetColor(float r, float g, float b);
	void SetCellSize(float cellSize);

private:

    int x_, y_; // 盤面座標
    	float cellSize_; // セルのサイズ
	float r_, g_, b_; // 色

	void UpdateTransform(void);

};
