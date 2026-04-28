#pragma once
#include "../ActorBase.h"

class Board : public ActorBase
{
public:

	// 定数
	static constexpr VECTOR DEFAULT_POSITION = { 0.0f, -50.0f, 0.0f };
	static constexpr VECTOR DEFAULT_SCALE = { 0.25f, 0.25f, 0.25f };
	static constexpr VECTOR DEFAULT_ROTATION = { 0.0f, 0.0f, 0.0f };

	// コンストラクタ
	Board(void);
	// デストラクタ
	virtual ~Board(void);
	virtual void Init(void) override;
	virtual void Update(void) override;
	virtual void Draw(void) override;

	void SetPosition(const VECTOR& pos);

private:

	// 描画用変数


};
