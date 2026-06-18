#include "../../../Utility/AsoUtility.h"
#include "MiniShogiPiece.h"
#include "MiniShogiGrid.h"


namespace
{
	constexpr float DEFAULT_PIECE_HEIGHT = 15.0f;
	constexpr float DEFAULT_PIECE_SCALE = 0.25f;
}

MiniShogiPiece::MiniShogiPiece(void)
	:
	boardX_(0),
	boardY_(0),
	boardOffset_(VGet(0.0f, 0.0f, 0.0f)),
	cellSize_(100.0f),
	pieceHeight_(DEFAULT_PIECE_HEIGHT),
	isVisible_(false)
{

	piece_=
	{
		PieceType::NONE,
		false,
		false
	};

}

MiniShogiPiece::~MiniShogiPiece(void)
{
	if (transform_.modelId >= 0)
	{
		MV1DeleteModel(transform_.modelId);
		transform_.modelId = -1;
	}
}

void MiniShogiPiece::Init(void)
{
	RefreshModel();

	transform_.scl = { DEFAULT_PIECE_SCALE,DEFAULT_PIECE_SCALE ,DEFAULT_PIECE_SCALE };

	transform_.Update();
}

void MiniShogiPiece::Update(void)
{
	transform_.Update();
}

void MiniShogiPiece::Draw(void)
{
	if (piece_.type_ == PieceType::NONE) return;

	if (transform_.modelId < 0) return;

	VECTOR pos = GetWorldPosition();

	transform_.pos = { pos.x,pos.y,pos.z };
	transform_.Update();

	MV1SetPosition(transform_.modelId, transform_.pos);
	MV1SetScale(transform_.modelId, transform_.scl);

	if (piece_.isPlayer_)
	{
		MV1SetRotationXYZ(transform_.modelId, VGet(0.0f, DX_PI_F, 0.0f));
	}
	else
	{
		MV1SetRotationXYZ(transform_.modelId, VGet(0.0f, 0.0f, 0.0f));
	}

	if (!isVisible_) return;

	MV1DrawModel(transform_.modelId);

}

void MiniShogiPiece::SetBoardCell(int x, int y)
{
	boardX_ = x;
	boardY_ = y;
}

void MiniShogiPiece::SetBoardOffset(VECTOR offset)
{
	boardOffset_ = offset;
}

void MiniShogiPiece::SetCellSize(float size)
{
	cellSize_ = size;
}

void MiniShogiPiece::SetPiece(const Piece& piece)
{
	piece_ = piece;
}

void MiniShogiPiece::SetModelHandle(int modelHandle)
{
	transform_.SetModel(modelHandle);
	RefreshModel();
}

void MiniShogiPiece::SetVisible(bool flag)
{
	isVisible_ = flag;
}

bool MiniShogiPiece::IsVisble(void) const
{
	return isVisible_;
}

void MiniShogiPiece::RefreshModel(void)
{
	if (transform_.modelId < 0)
	{
		return;
	}
}

VECTOR MiniShogiPiece::GetWorldPosition(void)
{
	transform_.pos.x = boardOffset_.x + (static_cast<float>(boardX_) * cellSize_);
	transform_.pos.z = boardOffset_.z + (static_cast<float>(boardY_) * cellSize_);
	transform_.pos.y = pieceHeight_;

	return transform_.pos;
}

void MiniShogiPiece::SetRotationY(float y)
{
	transform_.quaRotLocal = Quaternion::Euler({ AsoUtility::Deg2RadF(transform_.quaRot.x),
		AsoUtility::Deg2RadF(y),AsoUtility::Deg2RadF(transform_.quaRot.z) });
}