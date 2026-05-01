#pragma once

struct Player
{
	// プレイヤーの位置
	int x_, y_;

	// 色
	unsigned int color_;

	// プレイヤーの向き
	int forwardDirX_, forwardDirY_;
	int rightDirX_, rightDirY_;
};
