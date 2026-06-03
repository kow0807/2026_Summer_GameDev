#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/Setting.h"
#include "../../Application.h"
#include "Reversi.h"

Reversi::Reversi(void)
{
	Init();
}

Reversi::~Reversi(void)
{
}

void Reversi::Init(void)
{
	gameState_ = GameState::GO;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			board_[y][x] = EMPTY;
		}
	}

	board_[3][3] = WHITE;
	board_[4][4] = WHITE;
	board_[3][4] = BLACK;
	board_[4][3] = BLACK;

	cursorX_ = 3;
	cursorY_ = 3;

	playerTurn_ = true;

	blackCount_ = 2;
	whiteCount_ = 2;

	cpuThinkTimer_ = 0;
	cpuMoveCount_ = 0;
}

void Reversi::Update(void)
{
	UpdateStoneCount();
	InputManager& ins = InputManager::GetInstance();

	if (gameState_ != GameState::GO)
	{
		return;
	}

	if (playerTurn_)
	{
		if (ins.IsTrgDown(KEY_INPUT_LEFT))
		{
			cursorX_--;
		}

		if (ins.IsTrgDown(KEY_INPUT_RIGHT))
		{
			cursorX_++;
		}

		if (ins.IsTrgDown(KEY_INPUT_UP))
		{
			cursorY_--;
		}

		if (ins.IsTrgDown(KEY_INPUT_DOWN))
		{
			cursorY_++;
		}

		cursorX_ = max(0, min(7, cursorX_));
		cursorY_ = max(0, min(7, cursorY_));

		if (ins.IsTrgDown(KEY_INPUT_RETURN))
		{
			if (CanPlace(cursorX_, cursorY_, BLACK))
			{
				FlipStone(cursorX_, cursorY_, BLACK);

				playerTurn_ = false;

				cpuThinkTimer_ = GetRand(60) + 30;
			}
		}
	}
	else
	{
		if (cpuThinkTimer_ > 0)
		{
			cpuThinkTimer_--;
		}
		else
		{
			CPUAction();
		}
	}
}

void Reversi::Draw(void)
{
	int startX = 400;
	int startY = 80;
	int size = 64;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			DrawBox(
				startX + x * size,
				startY + y * size,
				startX + (x + 1) * size,
				startY + (y + 1) * size,
				GetColor(0, 120, 0),
				TRUE);

			DrawBox(
				startX + x * size,
				startY + y * size,
				startX + (x + 1) * size,
				startY + (y + 1) * size,
				GetColor(0, 0, 0),
				FALSE);

			if (board_[y][x] == BLACK)
			{
				DrawCircle(
					startX + x * size + size / 2,
					startY + y * size + size / 2,
					25,
					GetColor(0, 0, 0),
					TRUE);
			}
			else if (board_[y][x] == WHITE)
			{
				DrawCircle(
					startX + x * size + size / 2,
					startY + y * size + size / 2,
					25,
					GetColor(255, 255, 255),
					TRUE);
			}

			if (playerTurn_ &&
				CanPlace(x, y, BLACK))
			{
				DrawCircle(
					startX + x * size + size / 2,
					startY + y * size + size / 2,
					6,
					GetColor(255, 255, 0),
					TRUE);
			}
		}
	}

	DrawBox(
		startX + cursorX_ * size,
		startY + cursorY_ * size,
		startX + (cursorX_ + 1) * size,
		startY + (cursorY_ + 1) * size,
		GetColor(255, 0, 0),
		FALSE);
}

void Reversi::DrawUI(void)
{
	// サイドパネル
	DrawBox(
		30,
		30,
		330,
		650,
		GetColor(20, 20, 20),
		TRUE);

	DrawBox(
		30,
		30,
		330,
		650,
		GetColor(0, 180, 255),
		FALSE);

	// タイトル
	DrawString(
		110,
		50,
		"REVERSI",
		GetColor(0, 220, 255));

	// プレイヤー情報
	DrawString(
		50,
		120,
		"PLAYER",
		GetColor(255, 255, 255));

	DrawCircle(
		200,
		130,
		15,
		GetColor(0, 0, 0),
		TRUE);

	DrawString(
		225,
		120,
		"BLACK",
		GetColor(255, 255, 255));

	// CPU情報
	DrawString(
		50,
		180,
		"CPU",
		GetColor(255, 255, 255));

	DrawCircle(
		200,
		190,
		15,
		GetColor(255, 255, 255),
		TRUE);

	DrawString(
		225,
		180,
		"WHITE",
		GetColor(255, 255, 255));

	// 石数
	DrawLine(
		50,
		240,
		300,
		240,
		GetColor(0, 180, 255));

	DrawFormatString(
		50,
		270,
		GetColor(255, 255, 255),
		"BLACK : %d",
		blackCount_);

	DrawFormatString(
		50,
		310,
		GetColor(255, 255, 255),
		"WHITE : %d",
		whiteCount_);

	// ターン表示
	DrawLine(
		50,
		380,
		300,
		380,
		GetColor(0, 180, 255));

	if (playerTurn_)
	{
		DrawString(
			50,
			410,
			"YOUR TURN",
			GetColor(0, 255, 100));
	}
	else
	{
		DrawString(
			50,
			410,
			"CPU TURN",
			GetColor(255, 180, 0));
	}

	// CPU思考中
	if (!playerTurn_ && cpuThinkTimer_ > 0)
	{
		DrawString(
			50,
			450,
			"CPU Thinking...",
			GetColor(255, 255, 0));
	}

	// 操作説明
	DrawLine(
		50,
		520,
		300,
		520,
		GetColor(0, 180, 255));

	DrawString(
		50,
		550,
		"Arrow Key : Move",
		GetColor(200, 200, 200));

	DrawString(
		50,
		580,
		"Enter : Place Stone",
		GetColor(200, 200, 200));
}

void Reversi::Reset(void)
{
	Init();
}

bool Reversi::CanPlace(int x, int y, Stone stone)
{
	if (board_[y][x] != EMPTY)
	{
		return false;
	}

	Stone enemy = (stone == BLACK) ? WHITE : BLACK;

	int dirX[8] = { -1,0,1,-1,1,-1,0,1 };
	int dirY[8] = { -1,-1,-1,0,0,1,1,1 };

	for (int dir = 0; dir < 8; dir++)
	{
		int nx = x + dirX[dir];
		int ny = y + dirY[dir];

		bool foundEnemy = false;

		while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8)
		{
			if (board_[ny][nx] == enemy)
			{
				foundEnemy = true;
			}
			else if (board_[ny][nx] == stone)
			{
				if (foundEnemy)
				{
					return true;
				}
				break;
			}
			else
			{
				break;
			}

			nx += dirX[dir];
			ny += dirY[dir];
		}
	}

	return false;
}

void Reversi::FlipStone(int x, int y, Stone stone)
{
	Stone enemy = (stone == BLACK) ? WHITE : BLACK;

	int dirX[8] = { -1,0,1,-1,1,-1,0,1 };
	int dirY[8] = { -1,-1,-1,0,0,1,1,1 };

	board_[y][x] = stone;

	for (int dir = 0; dir < 8; dir++)
	{
		int nx = x + dirX[dir];
		int ny = y + dirY[dir];

		int count = 0;

		while (nx >= 0 && nx < 8 &&
			ny >= 0 && ny < 8 &&
			board_[ny][nx] == enemy)
		{
			count++;
			nx += dirX[dir];
			ny += dirY[dir];
		}

		if (count > 0 &&
			nx >= 0 && nx < 8 &&
			ny >= 0 && ny < 8 &&
			board_[ny][nx] == stone)
		{
			for (int i = 1; i <= count; i++)
			{
				board_[y + dirY[dir] * i]
					[x + dirX[dir] * i] = stone;
			}
		}
	}
}

int Reversi::CountFlip(int x, int y, Stone stone)
{
	if (!CanPlace(x, y, stone))
	{
		return 0;
	}

	Stone enemy = (stone == BLACK) ? WHITE : BLACK;

	int dirX[8] = { -1,0,1,-1,1,-1,0,1 };
	int dirY[8] = { -1,-1,-1,0,0,1,1,1 };

	int total = 0;

	for (int dir = 0; dir < 8; dir++)
	{
		int nx = x + dirX[dir];
		int ny = y + dirY[dir];

		int count = 0;

		while (nx >= 0 && nx < 8 &&
			ny >= 0 && ny < 8 &&
			board_[ny][nx] == enemy)
		{
			count++;
			nx += dirX[dir];
			ny += dirY[dir];
		}

		if (count > 0 &&
			nx >= 0 && nx < 8 &&
			ny >= 0 && ny < 8 &&
			board_[ny][nx] == stone)
		{
			total += count;
		}
	}

	return total;
}

int Reversi::EvaluateMove(int x, int y)
{
	if (!CanPlace(x, y, WHITE))
	{
		return -999999;
	}

	int score = 0;

	// 角
	if ((x == 0 && y == 0) ||
		(x == 7 && y == 0) ||
		(x == 0 && y == 7) ||
		(x == 7 && y == 7))
	{
		score += 100000;
	}

	// Xマス
	if ((x == 1 && y == 1) ||
		(x == 6 && y == 1) ||
		(x == 1 && y == 6) ||
		(x == 6 && y == 6))
	{
		score -= 50000;
	}

	// Cマス
	if ((x == 0 && y == 1) ||
		(x == 1 && y == 0) ||
		(x == 6 && y == 0) ||
		(x == 7 && y == 1) ||
		(x == 0 && y == 6) ||
		(x == 1 && y == 7) ||
		(x == 6 && y == 7) ||
		(x == 7 && y == 6))
	{
		score -= 30000;
	}

	// 辺
	if (x == 0 || x == 7 ||
		y == 0 || y == 7)
	{
		score += 3000;
	}

	Stone backup[8][8];

	for (int yy = 0; yy < 8; yy++)
	{
		for (int xx = 0; xx < 8; xx++)
		{
			backup[yy][xx] = board_[yy][xx];
		}
	}

	FlipStone(x, y, WHITE);

	// 相手の合法手を減らす
	int playerMoveCount = 0;

	for (int yy = 0; yy < 8; yy++)
	{
		for (int xx = 0; xx < 8; xx++)
		{
			if (CanPlace(xx, yy, BLACK))
			{
				playerMoveCount++;
			}
		}
	}

	score -= playerMoveCount * 500;

	// 自分の合法手を増やす
	int cpuMoveCount = 0;

	for (int yy = 0; yy < 8; yy++)
	{
		for (int xx = 0; xx < 8; xx++)
		{
			if (CanPlace(xx, yy, WHITE))
			{
				cpuMoveCount++;
			}
		}
	}

	score += cpuMoveCount * 300;

	// 序盤は石を取り過ぎない
	int flip = CountFlip(x, y, WHITE);

	if (cpuMoveCount_ < 8)
	{
		score -= flip * 50;
	}
	else
	{
		score += flip * 50;
	}

	// 盤面復元
	for (int yy = 0; yy < 8; yy++)
	{
		for (int xx = 0; xx < 8; xx++)
		{
			board_[yy][xx] = backup[yy][xx];
		}
	}

	return score;
}

bool Reversi::HasValidMove(Stone stone)
{
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (CanPlace(x, y, stone))
			{
				return true;
			}
		}
	}

	return false;
}

void Reversi::CPUAction()
{
	int bestX = -1;
	int bestY = -1;
	int bestScore = -999999;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (!CanPlace(x, y, WHITE))
			{
				continue;
			}

			int score = EvaluateMove(x, y);

			if (score > bestScore)
			{
				bestScore = score;
				bestX = x;
				bestY = y;
			}
		}
	}

	if (bestX != -1)
	{
		FlipStone(bestX, bestY, WHITE);
		cpuMoveCount_++;
	}

	playerTurn_ = true;
}

void Reversi::UpdateStoneCount()
{
	blackCount_ = 0;
	whiteCount_ = 0;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (board_[y][x] == BLACK)
			{
				blackCount_++;
			}
			else if (board_[y][x] == WHITE)
			{
				whiteCount_++;
			}
		}
	}
}