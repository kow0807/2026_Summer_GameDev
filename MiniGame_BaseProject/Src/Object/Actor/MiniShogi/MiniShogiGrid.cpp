#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "MiniShogiGrid.h"

namespace
{
	constexpr int BOARD_SIZE = 5;
	constexpr float GRID_HEIGHT_MIN = 0.0f;
	constexpr float GRID_HEIGHT_MAX = 6.0f;
	constexpr float CELL_MARGIN = 4.0f;
}

MiniShogiGrid::MiniShogiGrid(void)
	:
	boardOffset_(VGet(0.0f, 0.0f, 0.0f)),
	cellSize_(100.0f)
{
}

MiniShogiGrid::~MiniShogiGrid(void)
{
}

void MiniShogiGrid::Init(void)
{
}

void MiniShogiGrid::Update(void)
{
}

void MiniShogiGrid::Draw(void)
{
	for (int y = 0; y < BOARD_SIZE; y++)
	{
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			VECTOR center = GetCellCenter(x, y);

			VECTOR minPos = VGet(
				center.x - (cellSize_ * 0.5f) + CELL_MARGIN,
				GRID_HEIGHT_MIN,
				center.z - (cellSize_ * 0.5f) + CELL_MARGIN);

			VECTOR maxPos = VGet(
				center.x + (cellSize_ * 0.5f) - CELL_MARGIN,
				GRID_HEIGHT_MAX,
				center.z + (cellSize_ * 0.5f) - CELL_MARGIN);

			AsoUtility::DrawBox3D(
				minPos,
				maxPos,
				GetColor(180, 140, 80),
				FALSE);
		}
	}
}

void MiniShogiGrid::SetBoardOffset(VECTOR offset)
{
	boardOffset_ = offset;
}

void MiniShogiGrid::SetCellSize(float size)
{
	cellSize_ = size;
}

VECTOR MiniShogiGrid::GetCellCenter(int x, int y) const
{
	float posX = boardOffset_.x + (static_cast<float>(x) * cellSize_);
	float posZ = boardOffset_.z + (static_cast<float>(y) * cellSize_);

	return VGet(posX, 0.0f, posZ);
}