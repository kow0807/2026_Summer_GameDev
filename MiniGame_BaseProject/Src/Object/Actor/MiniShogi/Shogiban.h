#pragma once
#include "../ActorBase.h"
class Shogiban :
    public ActorBase
{
public:

    Shogiban(void);
    ~Shogiban(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:

};