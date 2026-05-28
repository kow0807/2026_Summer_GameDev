#pragma once
#include "../ActorBase.h"
class Triangle :
    public ActorBase
{
public:

	static constexpr VECTOR DEFAULT_SCALE = { 0.05f,0.05f,0.05f };

    Triangle(void);
    ~Triangle(void);
    void Init(void) override;
    void Update(void) override;
	void Draw(void) override;

	// z座標は固定で、x,yのみ変更する想定
	void SetPositon(float x,float z);

	// 回転角度変更
	void SetRotation(VECTOR rot);

	// 色変更
	void SetColor(float r, float g, float b);

private:

    float r_, g_, b_; // 色

};

