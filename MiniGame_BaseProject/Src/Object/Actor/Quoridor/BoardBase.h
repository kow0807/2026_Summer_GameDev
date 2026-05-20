#pragma once
#include "../ActorBase.h"
class BoardBase :
    public ActorBase
{
public:

    static constexpr VECTOR DEFAULT_SCALE = { 3.0f,1.0f,3.0f };

    BoardBase(void);
    virtual ~BoardBase(void);
    virtual void Init(void) override;
    virtual void Update(void) override;
	virtual void Draw(void) override;

private:

};

