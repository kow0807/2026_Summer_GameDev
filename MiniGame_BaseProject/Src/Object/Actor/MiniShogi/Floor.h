#pragma once
#include "../ActorBase.h"
class Floor :
    public ActorBase
{
public:

	float center = (9 - 1) * 50.0f * 0.5;

	// 定数
	static constexpr VECTOR DEFAULT_POSITION = { 250.0f, -260.0f, 250.0f };
	static constexpr VECTOR DEFAULT_SCALE = { 15.0f,5.0f, 15.0f };
	static constexpr VECTOR DEFAULT_ROTATION = { 0.0f, 0.0f, 0.0f };

	// コンストラクタ
	Floor(void);
	// デストラクタ
	virtual ~Floor(void);
	virtual void Init(void) override;
	virtual void Update(void) override;
	virtual void Draw(void) override;

	void SetPosition(const VECTOR& pos);

private:


};

