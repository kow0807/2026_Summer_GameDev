#pragma once
#include "GameBase.h"
class Quoridor : public GameBase
{
public:
	Quoridor(void);
	~Quoridor(void);
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;
	void Reset(void) override;

private:

};

