#pragma once
#include "../ActorBase.h"
class Shogiban :
    public ActorBase
{
public:

    static constexpr VECTOR DEFAULT_SCALE = { 0.5f,0.5f,0.5f };

    Shogiban(void);
    ~Shogiban(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:

};