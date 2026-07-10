#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Application.h"
#include "Reversi.h"

Reversi::Reversi(void)
{
	Init();
}

Reversi::~Reversi(void)
{
	DeleteFontToHandle(yuMinchoFontHandle_);
}

void Reversi::Init(void)
{
	yuMinchoFontHandle_ =
		CreateFontToHandle(
			"游明朝",
			16,
			3);
	backImg_ = resMng_.Load(ResourceManager::SRC::REVERSI_BACK).handleId_;

	isBgm_ = true;
	bgm_ = resMng_.Load(ResourceManager::SRC::REVERSI_BGM).handleId_;
	ChangeVolumeSoundMem(190, bgm_);
	pieceSe_ = resMng_.Load(ResourceManager::SRC::REVERSI_PIECE_SE).handleId_;
	isResult_ = true;
	clearSe_ = resMng_.Load(ResourceManager::SRC::REVERSI_CLEAR_SE).handleId_;
	overSe_ = resMng_.Load(ResourceManager::SRC::REVERSI_OVER_SE).handleId_;
	isSkip_ = true;
	skipSe_ = resMng_.Load(ResourceManager::SRC::REVERSI_SKIP_SE).handleId_;
	cursorSe_ = resMng_.Load(ResourceManager::SRC::REVERSI_MOVE_SE).handleId_;
	ChangeVolumeSoundMem(255, skipSe_);

	menuSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MENU_SE).handleId_;
	cancelSe_ = resMng_.Load(ResourceManager::SRC::SELECT_CANCEL_SE).handleId_;
	moveSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MOVE_SE).handleId_;
	decideSEH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_DICIDE_SE).handleId_;

	isPause_ = false;
	pauseScreenHandle_ = MakeScreen(
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		TRUE);
	pauseX_ = -320.0f;
	pauseSelect_ = 0;

	explanationFontHandle_ = CreateFontToHandle(
		"游明朝",
		18,
		3);

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

	skipMessage_ = false;
	skipTimer_ = 0;
	playerSkipped_ = false;
	resultTimer_ = 0;
}

void Reversi::Update(void)
{
	if (PauseUpdate())
	{
		return;
	}

	if (isBgm_)
	{
		PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
		isBgm_ = false;
	}
	UpdateStoneCount();
	
	if (gameState_ == GameState::RESULT)
	{
		resultTimer_++;

		if (resultTimer_ >= 240)
		{
			isReturn_ = true;
		}

		return;
	}

	InputManager& ins = InputManager::GetInstance();

	if (gameState_ != GameState::GO)
	{
		return;
	}

	if (!HasValidMove(BLACK) &&
		!HasValidMove(WHITE))
	{
		StopSoundMem(bgm_);
		gameState_ = GameState::RESULT;
	}

	if (skipTimer_ > 0)
	{
		if (isSkip_)
		{
			PlaySoundMem(skipSe_, DX_PLAYTYPE_BACK);
			isSkip_ = false;
		}

		skipTimer_--;

		if (skipTimer_ <= 0)
		{
			skipMessage_ = false;
			isSkip_ = true;
		}

		return;
	}

	if (playerTurn_)
	{
		if (!HasValidMove(BLACK))
		{
			playerTurn_ = false;

			skipMessage_ = true;
			playerSkipped_ = true;
			skipTimer_ = 120;

			cpuThinkTimer_ = 60;

			return;
		}

		if (ins.IsTrgDown(KEY_INPUT_LEFT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_LEFT))
		{
			PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK);
			cursorX_--;

			if (cursorX_ < 0)
			{
				cursorX_ = 7;
			}
		}

		if (ins.IsTrgDown(KEY_INPUT_RIGHT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_RIGHT))
		{
			PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK);
			cursorX_++;

			if (cursorX_ > 7)
			{
				cursorX_ = 0;
			}
		}

		if (ins.IsTrgDown(KEY_INPUT_UP) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP))
		{
			PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK);
			cursorY_--;

			if (cursorY_ < 0)
			{
				cursorY_ = 7;
			}
		}

		if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))
		{
			PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK);
			cursorY_++;

			if (cursorY_ > 7)
			{
				cursorY_ = 0;
			}
		}

		if (ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
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

	int screenX = Application::SCREEN_SIZE_X;
	int screenY = Application::SCREEN_SIZE_Y;

	int centerX = screenX / 2;
	int centerY = screenY / 2;

	// 背景
	DrawRotaGraph(centerX, centerY, 0.7, 0.0, backImg_, true);

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
		580,
		GetColor(20, 20, 20),
		TRUE);

	DrawBox(
		30,
		30,
		330,
		580,
		GetColor(0, 180, 255),
		FALSE);

	// タイトル
	DrawStringToHandle(
		110,
		50,
		"REVERSI",
		GetColor(0, 220, 255),
		yuMinchoFontHandle_);

	// プレイヤー情報
	DrawStringToHandle(
		50,
		120,
		"PLAYER",
		GetColor(255, 255, 255),
		yuMinchoFontHandle_);

	DrawCircle(
		200,
		130,
		15,
		GetColor(0, 0, 0),
		TRUE);

	DrawStringToHandle(
		225,
		120,
		"BLACK",
		GetColor(255, 255, 255),
		yuMinchoFontHandle_);

	// CPU情報
	DrawStringToHandle(
		50,
		180,
		"CPU",
		GetColor(255, 255, 255),
		yuMinchoFontHandle_);

	DrawCircle(
		200,
		190,
		15,
		GetColor(255, 255, 255),
		TRUE);

	DrawStringToHandle(
		225,
		180,
		"WHITE",
		GetColor(255, 255, 255),
		yuMinchoFontHandle_);

	// 石数
	DrawLine(
		50,
		240,
		300,
		240,
		GetColor(0, 180, 255));

	DrawFormatStringToHandle(
		50,
		270,
		GetColor(255, 255, 255),
		yuMinchoFontHandle_,
		"BLACK : %d",
		blackCount_);

	DrawFormatStringToHandle(
		50,
		310,
		GetColor(255, 255, 255),
		yuMinchoFontHandle_,
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
		DrawStringToHandle(
			50,
			410,
			"YOUR TURN",
			GetColor(0, 255, 100),
			yuMinchoFontHandle_);
	}
	else
	{
		DrawStringToHandle(
			50,
			410,
			"CPU TURN",
			GetColor(255, 180, 0),
			yuMinchoFontHandle_);
	}

	// CPU思考中
	if (!playerTurn_ && cpuThinkTimer_ > 0)
	{
		DrawStringToHandle(
			50,
			450,
			"CPU Thinking...",
			GetColor(255, 255, 0),
			yuMinchoFontHandle_);
	}

	DrawLine(
		50,
		520,
		300,
		520,
		GetColor(0, 180, 255));

	if (skipMessage_ && gameState_ != GameState::RESULT)
	{
		// 半透明の背景
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
		DrawBox(
			380,
			300,
			900,
			480,
			GetColor(20, 20, 20),
			TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 白い枠
		DrawBox(
			380,
			300,
			900,
			480,
			GetColor(255, 255, 255),
			FALSE);

		const int centerX = (380 + 900) / 2;

		// タイトル
		const char* title = "SKIP";
		int titleWidth = GetDrawStringWidthToHandle(
			title,
			static_cast<int>(strlen(title)),
			yuMinchoFontHandle_);

		DrawStringToHandle(
			centerX - titleWidth / 2,
			325,
			title,
			GetColor(255, 255, 255),
			yuMinchoFontHandle_);

		// 区切り線
		DrawLine(
			430,
			375,
			850,
			375,
			GetColor(180, 180, 180));

		// メッセージ
		const char* skipText;
		int skipColor;

		if (playerSkipped_)
		{
			skipText = "PLAYER SKIP!";
			skipColor = GetColor(255, 120, 120);
		}
		else
		{
			skipText = "CPU SKIP!";
			skipColor = GetColor(255, 255, 0);
		}

		int textWidth = GetDrawStringWidthToHandle(
			skipText,
			static_cast<int>(strlen(skipText)),
			yuMinchoFontHandle_);

		DrawStringToHandle(
			centerX - textWidth / 2,
			405,
			skipText,
			skipColor,
			yuMinchoFontHandle_);
	}

	if (gameState_ == GameState::RESULT)
	{
		// 半透明の背景
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
		DrawBox(
			300,
			140,
			1000,
			580,
			GetColor(20, 20, 20),
			TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 白い枠
		DrawBox(
			300,
			140,
			1000,
			580,
			GetColor(255, 255, 255),
			FALSE);

		// ウィンドウ中央
		const int centerX = (300 + 1000) / 2;

		// タイトル（中央揃え）
		const char* titleText = "RESULT";
		int titleWidth = GetDrawStringWidthToHandle(
			titleText,
			static_cast<int>(strlen(titleText)),
			yuMinchoFontHandle_);

		DrawStringToHandle(
			centerX - titleWidth / 2,
			170,
			titleText,
			GetColor(255, 255, 255),
			yuMinchoFontHandle_);

		// 区切り線
		DrawLine(
			360,
			230,
			940,
			230,
			GetColor(180, 180, 180));

		// 石の数
		DrawFormatStringToHandle(
			420,
			270,
			GetColor(40, 220, 255),
			yuMinchoFontHandle_,
			"PLAYER : %d",
			blackCount_);

		DrawFormatStringToHandle(
			740,
			270,
			GetColor(255, 120, 120),
			yuMinchoFontHandle_,
			"CPU    : %d",
			whiteCount_);

		// 勝敗表示
		const char* resultText = nullptr;
		int resultColor = GetColor(255, 255, 255);

		if (blackCount_ > whiteCount_)
		{
			if (isResult_)
			{
				PlaySoundMem(clearSe_, DX_PLAYTYPE_BACK);
				isResult_ = false;
			}

			resultText = "PLAYER WIN!";
			resultColor = GetColor(0, 255, 120);
		}
		else if (whiteCount_ > blackCount_)
		{
			if (isResult_)
			{
				PlaySoundMem(overSe_, DX_PLAYTYPE_BACK);
				isResult_ = false;
			}

			resultText = "CPU WIN!";
			resultColor = GetColor(255, 80, 80);
		}
		else
		{
			if (isResult_)
			{
				PlaySoundMem(overSe_, DX_PLAYTYPE_BACK);
				isResult_ = false;
			}

			resultText = "DRAW";
			resultColor = GetColor(255, 230, 0);
		}

		// 勝敗表示（中央揃え）
		int resultWidth = GetDrawStringWidthToHandle(
			resultText,
			static_cast<int>(strlen(resultText)),
			yuMinchoFontHandle_);

		DrawStringToHandle(
			centerX - resultWidth / 2,
			360,
			resultText,
			resultColor,
			yuMinchoFontHandle_);
	}

	// パネルが見えている間だけ描画
	if (isPause_ || pauseX_ > -320.0f)
	{
		PauseDraw();
	}
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
	PlaySoundMem(pieceSe_, DX_PLAYTYPE_BACK);

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
	if (!HasValidMove(WHITE))
	{
		playerTurn_ = true;

		skipMessage_ = true;
		playerSkipped_ = false;
		skipTimer_ = 120;

		return;
	}

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

bool Reversi::PauseUpdate(void)
{
	InputManager& ins = InputManager::GetInstance();

	//--------------------------------------
	// Escapeで開閉
	//--------------------------------------
	if (ins.IsTrgDown(KEY_INPUT_ESCAPE) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::SEVEN))
	{
		isPause_ = !isPause_;

		if (isPause_)
		{
			// ポーズ直前の画面を保存
			GetDrawScreenGraph(
				0,
				0,
				Application::SCREEN_SIZE_X,
				Application::SCREEN_SIZE_Y,
				pauseScreenHandle_);

			PlaySoundMem(menuSe_, DX_PLAYTYPE_BACK);
			StopSoundMem(bgm_);
			StopSoundMem(pieceSe_);
			StopSoundMem(clearSe_);
			StopSoundMem(overSe_);
			StopSoundMem(skipSe_);
			StopSoundMem(cursorSe_);
		}
		else
		{
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
			PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP, false);
			PlaySoundMem(pieceSe_, DX_PLAYTYPE_BACK, false);
			PlaySoundMem(clearSe_, DX_PLAYTYPE_BACK, false);
			PlaySoundMem(overSe_, DX_PLAYTYPE_BACK, false);
			PlaySoundMem(skipSe_, DX_PLAYTYPE_BACK, false);
			PlaySoundMem(cursorSe_, DX_PLAYTYPE_BACK, false);
		}

		// 開いた・閉じた瞬間は入力を消費
		return true;
	}

	//--------------------------------------
	// スライドアニメーション
	//--------------------------------------
	float targetX = isPause_ ? 0.0f : -320.0f;
	pauseX_ += (targetX - pauseX_) * 0.2f;

	//--------------------------------------
	// ポーズ中でなければゲームへ入力を渡す
	//--------------------------------------
	if (!isPause_)
		return false;

	//--------------------------------------
	// カーソル移動
	//--------------------------------------
	if (ins.IsTrgDown(KEY_INPUT_UP) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);

		pauseSelect_--;

		if (pauseSelect_ < 0)
			pauseSelect_ = 2;
	}

	if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))
	{
		PlaySoundMem(moveSe_, DX_PLAYTYPE_BACK);

		pauseSelect_++;

		if (pauseSelect_ > 2)
			pauseSelect_ = 0;
	}

	//--------------------------------------
	// 決定
	//--------------------------------------
	if (ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		switch (pauseSelect_)
		{
		case 0:
			isPause_ = false;
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
			PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP, false);
			return true;    // このフレームはゲームへ入力を渡さない

		case 1:
			PlaySoundMem(decideSEH_, DX_PLAYTYPE_BACK);
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
			return true;

		case 2:
			SceneManager::GetInstance().SetGameEnd(true);
			return true;
		}
	}

	// ポーズ中は常に入力を消費
	return true;
}

void Reversi::PauseDraw(void)
{
	//--------------------------------------
	// 背景（ポーズ中だけ）
	//--------------------------------------
	if (!isPause_)
		return;


	//--------------------------------------
	// ポーズ直前の画面
	//--------------------------------------
	DrawGraph(
		0,
		0,
		pauseScreenHandle_,
		TRUE);


	//--------------------------------------
	// 暗いフィルター
	//--------------------------------------
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

	DrawBox(
		0,
		0,
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		GetColor(0, 0, 0),
		TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//--------------------------------------
	// パネルが画面外なら描画しない
	//--------------------------------------
	if (pauseX_ <= -320.0f)
		return;

	//--------------------------------------
	// パネル影
	//--------------------------------------
	DrawBox(
		(int)pauseX_ + 8,
		8,
		(int)pauseX_ + 308,
		Application::SCREEN_SIZE_Y,
		GetColor(20, 20, 20),
		TRUE);

	//--------------------------------------
	// パネル本体
	//--------------------------------------
	DrawBox(
		(int)pauseX_,
		0,
		(int)pauseX_ + 300,
		Application::SCREEN_SIZE_Y,
		GetColor(35, 35, 45),
		TRUE);

	//--------------------------------------
	// 枠
	//--------------------------------------
	DrawBox(
		(int)pauseX_ - 100,
		0,
		(int)pauseX_ + 300,
		Application::SCREEN_SIZE_Y,
		GetColor(0, 220, 255),
		FALSE);

	//--------------------------------------
	// タイトル
	//--------------------------------------
	DrawStringToHandle(
		(int)pauseX_ + 30,
		40,
		"PAUSE",
		GetColor(255, 255, 255),
		explanationFontHandle_);

	DrawLine(
		(int)pauseX_ + 30,
		75,
		(int)pauseX_ + 270,
		75,
		GetColor(0, 220, 255));

	//--------------------------------------
	// メニュー
	//--------------------------------------
	const char* menu[3] =
	{
		"ゲームに戻る",
		"タイトルに戻る",
		"ゲーム終了"
	};

	for (int i = 0; i < 3; i++)
	{
		int y = 150 + i * 80;

		if (i == pauseSelect_)
		{
			DrawBox(
				(int)pauseX_ + 20,
				y - 8,
				(int)pauseX_ + 280,
				y + 30,
				GetColor(0, 180, 220),
				TRUE);

			DrawBox(
				(int)pauseX_ + 20,
				y - 8,
				(int)pauseX_ + 28,
				y + 30,
				GetColor(255, 255, 255),
				TRUE);

			DrawStringToHandle(
				(int)pauseX_ + 45,
				y,
				menu[i],
				GetColor(255, 255, 255),
				explanationFontHandle_);
		}
		else
		{
			DrawStringToHandle(
				(int)pauseX_ + 45,
				y,
				menu[i],
				GetColor(180, 180, 180),
				explanationFontHandle_);
		}
	}

	//--------------------------------------
	// 操作説明
	//--------------------------------------
	DrawLine(
		(int)pauseX_ + 20,
		500,
		(int)pauseX_ + 280,
		500,
		GetColor(80, 80, 80));

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}