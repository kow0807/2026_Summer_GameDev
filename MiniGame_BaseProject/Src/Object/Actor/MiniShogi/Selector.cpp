#include "Selector.h"

Selector::Selector(void)
    :
    isSelecting_(false),
    selectArea_(CursorArea::BOARD),
    selectX_(0),
    selectY_(0),
    selectHandIndex_(0),
    selectPieceType_(PieceType::NONE)
{
}

Selector::~Selector(void)
{
}

void Selector::Select(bool flag)
{
    isSelecting_ = flag;

    if (!flag)
    {
        moveList_.clear();
        selectPieceType_ = PieceType::NONE;
    }
}

bool Selector::IsSelecting(void) const
{
    return isSelecting_;
}

void Selector::SetSelectPositon(CursorArea area, int x, int y, int handIndex)
{
    selectArea_ = area;
    selectX_ = x;
    selectY_ = y;
    selectHandIndex_ = handIndex;
}

void Selector::SetSelectPieceType(PieceType type)
{
    selectPieceType_ = type;
}

void Selector::SetMoveList(const std::vector<MoveData>& moveList)
{
    moveList_ = moveList;
}

const std::vector<MoveData>& Selector::GetMoveList(void) const
{
    return moveList_;
}

bool Selector::IsMovePosition(int x, int y) const
{
    for (const auto& move : moveList_)
    {
        if (move.x_ == x
            && move.y_ == y)
        {
            return true;
        }
    }
    
    return false;
}

CursorArea Selector::GetSelectArea(void) const
{
    return selectArea_;
}

int Selector::GetSelectX(void) const
{
    return selectX_;
}

int Selector::GetSelectY(void) const
{
    return selectY_;
}

int Selector::GetSelectHandIndex(void) const
{
    return selectHandIndex_;
}

PieceType Selector::GetSelectPieceType(void) const
{
    return selectPieceType_;
}
