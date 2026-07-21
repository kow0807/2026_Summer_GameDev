#pragma once
#include "../ActorBase.h"
class Komadai :
    public ActorBase
{
public:

    // íËêî
    static constexpr VECTOR PLAYER_POSITION = { -430.0f, -250.0f, 90.0f };
    static constexpr VECTOR ENEMY_POSITION = { 430.0f, -250.0f, -90.0f };
    static constexpr VECTOR DEFAULT_POSITION = { 0.0f, -1000.0f, 0.0f };
    static constexpr VECTOR DEFAULT_SCALE = { 0.42f,0.2f, 0.42f };
    static constexpr VECTOR DEFAULT_ROTATION = { 0.0f, 0.0f, 0.0f };


    Komadai(void);
    ~Komadai(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

    void SetPosition(const VECTOR& pos);

private:


};

