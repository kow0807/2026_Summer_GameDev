#pragma once

#include "../ActorBase.h"

#include "Piece.h"

#include <DxLib.h>

class MiniShogiPiece
    :
    public ActorBase
{
public:

    MiniShogiPiece(void);
    ~MiniShogiPiece(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

    void SetBoardCell(int x, int y);
    void SetBoardOffset(VECTOR offset);

    void SetCellSize(float size);

    void SetPiece(const Piece& piece);
    void SetModelHandle(int modelHandle);

    void SetVisible(bool flag);
    bool IsVisble(void) const;

    void SetRotationY(float y);

    void SetRotationZ(float z);

    void SetWorldPosition(VECTOR position);

private:

    Piece piece_;

    int boardX_, boardY_;

    VECTOR boardOffset_;
    float cellSize_;

    float pieceHeight_;

    bool isVisible_;

    bool useWorldPosition_;
    VECTOR worldPosition_;

    void RefreshModel(void);

    VECTOR GetWorldPosition(void);
};

