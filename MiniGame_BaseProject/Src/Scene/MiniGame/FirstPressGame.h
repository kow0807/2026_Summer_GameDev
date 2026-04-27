#pragma once
#include "GameBase.h"


class FirstPressGame : public GameBase
{
public:
	FirstPressGame(void);
	~FirstPressGame(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;
	void Reset(void) override;

private:

};

