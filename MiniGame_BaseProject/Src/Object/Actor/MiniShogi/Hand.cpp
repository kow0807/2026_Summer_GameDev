#include "Hand.h"

Hand::Hand(void)
{
}

Hand::~Hand(void)
{
}

void Hand::AddPiece(PieceType type)
{
    int index = FindPieceIndex(type);

    if (index >= 0)
    {
        handPieceList_[index].count_++;

        return;
    }

    handPieceList_.push_back(
        {
            type,
            1
        }
    );
}

void Hand::RemovePiece(PieceType type)
{
    int index = FindPieceIndex(type);

    if (index < 0)
    {
        return;
    }

    handPieceList_[index].count_--;

    if (handPieceList_[index].count_ <= 0)
    {
        handPieceList_.erase(
            handPieceList_.begin() + index
        );
    }
}

bool Hand::HasPiece(PieceType type)
{
    return FindPieceIndex(type) >= 0;
}

bool Hand::IsEmpty(void) const
{
    return handPieceList_.empty();
}

int Hand::GetPieceCount(void) const
{
    return static_cast<int>(handPieceList_.size());
}

const HandPiece& Hand::GetPiece(int index) const
{
    return handPieceList_[index];
}

int Hand::FindPieceIndex(PieceType type) const
{
    for (int i = 0; i < static_cast<int>(handPieceList_.size()); i++)
    {
        if (handPieceList_[i].type_ == type)
        {
            return i;
        }
    }

    return -1;
}
