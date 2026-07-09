#include "Hand.h"

Hand::Hand(void)
{
}

Hand::~Hand(void)
{
}

void Hand::AddPiece(PieceType type)
{
    handPieceList_.push_back(
        {
            type
        }
    );
}

void Hand::RemovePiece(PieceType type)
{
    int index = FindIndex(type);

    if (index < 0)
    {
        return;
    }

    if (index >= 0)
    {
		handPieceList_.erase(handPieceList_.begin() + index);
    }
}

bool Hand::HasPiece(PieceType type) const
{
    return FindIndex(type) >= 0;
}

bool Hand::HasPiece(int index) const
{
    return index >= 0 && 
           index < static_cast<int>(handPieceList_.size());
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

const std::vector<HandPiece>& Hand::GetPieceList(void) const
{
	return handPieceList_;
}

int Hand::FindIndex(PieceType type) const
{
	// 手駒リストから指定された駒のインデックスを検索する
    for (int i = 0; i < static_cast<int>(handPieceList_.size()); i++)
    {
        if (handPieceList_[i].type_ == type)
        {
            return i;
        }
    }

    return -1;
}
