#pragma once
#include "../ActorBase.h"
class Komadai :
    public ActorBase
{
public:

    static constexpr VECTOR DEFAULT_SCALE = { 1.0f,1.0f,1.0f };

    Komadai(void);
    ~Komadai(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:


};

