#include "Selector.h"

Selector::Selector(void)
    :
    isSelecting_(false),
    selectArea_(CursorArea::BOARD),
    selectX_(0),
    selectY_(0),
    selectHandIndex_(0)
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

void Selector::SetMoveList(const std::vector<MoveData>& moveList)
{
    moveList_ = moveList;
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
