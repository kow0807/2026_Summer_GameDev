#pragma once
#include "GameBase.h"
class MiniShogi :
    public GameBase
{
public:

    MiniShogi(void);
    ~MiniShogi(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void DrawUI(void) override;
    void Reset(void) override;

private:

};

