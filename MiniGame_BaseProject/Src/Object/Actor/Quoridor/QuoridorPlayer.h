#pragma once

struct Player
{
	// プレイヤーの位置
	int x_, y_;

	// 色
	float r_, g_, b_;

	// プレイヤーの向き
	int forwardDirX_, forwardDirY_;
	int rightDirX_, rightDirY_;

	// 残り壁数
	int remainingWalls_;
};
