#include "HandActor.h"

#include "../../../Manager/ResourceManager.h"

#include "Hand.h"
#include "MiniShogiPiece.h"


HandActor::HandActor(Hand* hand, bool isPlayerSide)
	:
	hand_(hand),
	isPlayerSide_(isPlayerSide),
	origin_(VGet(0,0,0)),
	cellSize_(70.0f),
	resMng_(ResourceManager::GetInstance()),
	ouH_(-1),
	kinH_(-1),
	ginH_(-1),
	kakuH_(-1),
	hishaH_(-1),
	fuH_(-1)
{
}

HandActor::~HandActor(void)
{
}

void HandActor::Init(void)
{
	ouH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_OU);
	kinH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_KIN);
	ginH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_GIN);
	kakuH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_KAKU);
	hishaH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_HISHA);
	fuH_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MINISHOGI_FU);

	for (auto& piece : pieces_)
	{
		piece = std::make_unique<MiniShogiPiece>();

		piece->Init();
		piece->SetVisible(false);
	}
}

void HandActor::Update(void)
{
	SyncPieceActor();

	for (auto& piece : pieces_)
	{
		piece->Update();
	}
}

void HandActor::Draw(void)
{
	for (const auto& piece : pieces_)
	{
		piece->Draw();
	}
}

void HandActor::SetOrigin(const VECTOR& origin)
{
	origin_ = origin;
}

void HandActor::SetCellSize(float size)
{
	cellSize_ = size;
}

VECTOR HandActor::GetPieceWorldPosition(int index) const
{
	VECTOR pos = origin_;
	pos.z += index % 3 * cellSize_;
	return pos;
}

int HandActor::GetPieceCount(void) const
{
	if (hand_ == nullptr)
	{
		return 0;
	}

	return hand_->GetPieceCount();
}

void HandActor::SyncPieceActor(void)
{
	int pieceCount = hand_->GetPieceCount();

	for (int i = 0; i < pieceCount; i++)
	{
		const HandPiece& handPiece =
			hand_->GetPiece(i);

		auto& actor = pieces_[i];

		actor->SetVisible(true);

		Piece piece
		{
			handPiece.type_,
			isPlayerSide_,
			false
		};

		actor->SetPiece(piece);

		actor->SetWorldPosition(
			CalcWorldPosition(i)
		);

		actor->SetModelHandle(
			GetModelHandle(handPiece.type_)
		);
	}

	for (int i = pieceCount; i < MAX_HAND_PIECE; i++)
	{
		pieces_[i]->SetVisible(false);
	}
}

int HandActor::GetModelHandle(PieceType type) const
{
	switch (type)
	{
	case PieceType::OU:      return ouH_;
	case PieceType::KIN:     return kinH_;
	case PieceType::GIN:     return ginH_;
	case PieceType::KAKUGYO: return kakuH_;
	case PieceType::HISHA:   return hishaH_;
	case PieceType::FU:      return fuH_;
	default:                 return -1;
	}
}

VECTOR HandActor::CalcWorldPosition(int index) const
{
	int row = index % 3;
	int col = index / 3;

	float x;

	if (isPlayerSide_)
	{
		x = origin_.x + (col * cellSize_);
	}
	else
	{
		x = origin_.x - (col * cellSize_);
	}

	float z = origin_.z + (row * cellSize_);

	return VGet(x, 15.0f, z);
}