#pragma once
#include "../ActorBase.h"
class Shogiban :
    public ActorBase
{
public:

    static constexpr VECTOR DEFAULT_SCALE = { 1.35f,1.0f,1.35f };

    Shogiban(void);
    ~Shogiban(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:

};