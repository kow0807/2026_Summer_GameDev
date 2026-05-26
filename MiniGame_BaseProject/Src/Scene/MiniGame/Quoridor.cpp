#include <queue>
#include <fstream>
#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Object/Actor/Quoridor/Desk.h"
#include "../../Object/Actor/Quoridor/Board.h"
#include "../../Object/Actor/Quoridor/Wall.h"
#include "../../Object/Actor/Quoridor/PlayerPiece.h"
#include "../../Object/Actor/Quoridor/PythonAI.h"
#include "Quoridor.h"

// デバッグ用ログ出力関数
static void DbgLog(const std::string& msg)
{
	std::ofstream f("quoridor_debug.log", std::ios::app);
	f << msg << "\n";
}

Quoridor::Quoridor(void)
	:
	gameMode_(GAME_MODE::NONE),
	mode_(MODE::NONE),
	players_{},
	currentTurn_(0),
	isChangeTurn_(false),
	wallCursorX_(0),
	wallCursorY_(0),
	wallVertical_(true),
	isGameOver_(false),
	winner_(-1),
	isCpuThinking_(false),
	fontTitle_(-1),
	fontMain_(-1)
{
	DbgLog("=== Quoridor START ===");
}

Quoridor::~Quoridor(void)
{
}

void Quoridor::Init(void)
{
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::QUORIDOR);

	// 描画環境設定
	SetGlobalAmbientLight(GetColorF(0.55f, 0.55f, 0.55f, 1.0f));

	int lightHandle = CreateDirLightHandle(VNorm(VGet(-1.0f, -1.0f, 0.5f)));
	SetLightDifColorHandle(lightHandle, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));

	//SetUseShadowMap(TRUE);
	SetLightEnable(TRUE);

	// ゲームモードの設定
	gameMode_ = GAME_MODE::CPU;

	// プレイヤーの初期化
	players_[0] = { 4, 0, 1.0f, 0.31f, 0.31f, 0, 1, 1, 0, MAX_WALLS,{1.0f, 0.31f, 0.31f} };
	players_[1] = { 4, 8, 0.31f, 0.31f, 1.0f, 0, -1, -1, 0, MAX_WALLS, {0.31f, 0.31f, 1.0f} };

	// フォント
	fontTitle_ = CreateFontToHandle("游明朝", 48, -1, DX_FONTTYPE_ANTIALIASING);
	fontMain_ = CreateFontToHandle("游明朝", 24, -1, DX_FONTTYPE_ANTIALIASING);

	// オブジェクトの初期化
	desk_ = std::make_unique<Desk>();
	desk_->Init();
	desk_->SetPosition(Desk::DEFAULT_POSITION);

	board_ = std::make_unique<Board>();
	board_->Init();

	previewWall_ = std::make_unique<Wall>();
	previewWall_->SetCellSize(Board::CELL_SIZE);
	previewWall_->InitTransform();
	previewWall_->SetType(Wall::TYPE::VERTICAL);

	for (int i = 0; i < 2; ++i)
	{
		playerPieces_[i] = std::make_unique<PlayerPiece>();
		playerPieces_[i]->SetCellSize(Board::CELL_SIZE);
		playerPieces_[i]->Init();
		playerPieces_[i]->SetBoardPosition(players_[i].x_, players_[i].y_);
		playerPieces_[i]->SetColor(players_[i].r_, players_[i].g_, players_[i].b_);
	}

	RefreshMoveCandidates();

	pythonAI_ = std::make_unique<PythonAI>();
	bool ok = pythonAI_->Start(
		L"Python\\python.exe",
		L"Python\\ai\\ai.py"
	);

	if (ok)
	{
#ifdef _DEBUG
		DbgLog("[Init] PythonAI Start OK");
#endif
	}
	else
	{
		DbgLog("[Init] PythonAI Start FAILED -> PLAYER mode");
		gameMode_ = GAME_MODE::PLAYER;
	}
}

void Quoridor::Update(void)
{
	auto& ins = InputManager::GetInstance();
	if (isGameOver_)
	{
		if (ins.IsTrgUp(KEY_INPUT_R))
		{
			DbgLog("[Update] R key pressed. Restarting game!");
			Reset();
			return;
		}
	}

	if (isGameOver_) return;

	desk_->Update();

	if (players_[currentTurn_].remainingWalls_ <= 0 && mode_ == MODE::WALL)
	{
		mode_ = MODE::MOVE;
		WaitTimer(150);
		RefreshMoveCandidates();
	}

	if (gameMode_ == GAME_MODE::CPU && currentTurn_ == 1)
	{
		UpdateCpu();
	}
	else
	{
		UpdatePlayer();
	}

	for (int i = 0; i < 2; ++i)
	{
		playerPieces_[i]->SetBoardPosition(players_[i].x_, players_[i].y_);
	}

	Player& cur = players_[currentTurn_];
	if (cur.x_ < 0)             cur.x_ = 0;
	if (cur.x_ >= BOARD_SIZE)   cur.x_ = BOARD_SIZE - 1;
	if (cur.y_ < 0)             cur.y_ = 0;
	if (cur.y_ >= BOARD_SIZE)   cur.y_ = BOARD_SIZE - 1;

	winner_ = board_->CheckWinner(players_);
	if (winner_ >= 0)
	{
		isGameOver_ = true;
		return;
	}

	if (isChangeTurn_)
	{
		DbgLog("[Update] Turn change: " + std::to_string(currentTurn_) + " -> " + std::to_string((currentTurn_ + 1) % 2));
		DbgLog("[Update] CPU pos after move: x=" + std::to_string(players_[1].x_) + " y=" + std::to_string(players_[1].y_));
		currentTurn_ = (currentTurn_ + 1) % 2;
		isChangeTurn_ = false;
		RefreshMoveCandidates();
	}
}

void Quoridor::Draw(void)
{
	desk_->Draw();
	DrawBoard();
	DrawMoveCandidates();
	DrawPlayers();
	DrawWallAndGrid();

	if (mode_ == MODE::WALL)
	{
		bool canPlace =
			(players_[currentTurn_].remainingWalls_ > 0) &&
			board_->CanPlaceWall(wallCursorX_, wallCursorY_, wallVertical_);

		if (IsBlink())
		{
			previewWall_->DrawPreview(canPlace, players_[currentTurn_].wallColor_);
		}
	}
}

void Quoridor::DrawUI(void)
{
	if (isGameOver_)
	{
		DrawGameOver();
	
	}
	
	// 1. 現在の画面サイズを動的に取得
	int screenW, screenH;
	GetWindowSize(&screenW, &screenH); // もしフルスクリーンの場合は GetDrawScreenSize など

	// 2. 画面サイズ（特に縦幅）の変化を検知してフォントサイズを動的更新
	static int lastScreenH = 0;
	if (screenH != lastScreenH)
	{
		// 古いフォントがあれば削除
		if (fontTitle_ != -1) DeleteFontToHandle(fontTitle_);
		if (fontMain_ != -1)  DeleteFontToHandle(fontMain_);

		// 画面の高さに合わせてフォントサイズを動的に計算
		// 縦720のときに40と18になるように比率をかける
		int titleFontSize = (int)(40.0f * ((float)screenH / 720.0f));
		int mainFontSize = (int)(18.0f * ((float)screenH / 720.0f));

		// 最低サイズを保証（画面が極端に小さくなった時の対策）
		if (titleFontSize < 16) titleFontSize = 16;
		if (mainFontSize < 9)   mainFontSize = 9;

		fontTitle_ = CreateFontToHandle("游明朝", titleFontSize, 3, DX_FONTTYPE_ANTIALIASING_8X8);
		fontMain_ = CreateFontToHandle("游明朝", mainFontSize, 2, DX_FONTTYPE_ANTIALIASING_8X8);
		
		lastScreenH = screenH;
	}

	// 各種色の定義
	unsigned int colorWhite = GetColor(240, 230, 220);
	unsigned int colorGray = GetColor(150, 140, 130);
	unsigned int colorRed = GetColor(200, 80, 80);
	unsigned int colorBlue = GetColor(80, 80, 200);

	// 3. 座標を画面サイズの「割合（％）」で動的に計算
	// 左側の余白は画面横幅の約 4%
	int leftX = (int)(screenW * 0.04f);
	// 右側のプレイヤー情報は画面横幅の約 78%
	int rightX = (int)(screenW * 0.78f);
	// 行間（文字の上下の間隔）も画面の高さに比例させる
	int lineGap = (int)(screenH * 0.04f);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);

	// 左側のUI背景ボックス（タイトルから操作説明までを包む）
	DrawBox(
		(int)(screenW * 0.02f), (int)(screenH * 0.03f), // 左上
		(int)(screenW * 0.25f), (int)(screenH * 0.78f), // 右下
		GetColor(25, 20, 15), TRUE // 目標画面に合わせた暗い焦げ茶色
	);

	// 右側のUI背景ボックス（プレイヤー1・2の情報を包む）
	DrawBox(
		(int)(screenW * 0.76f), (int)(screenH * 0.14f), // 左上
		(int)(screenW * 0.98f), (int)(screenH * 0.52f), // 右下
		GetColor(25, 20, 15), TRUE // 暗い焦げ茶色
	);

	// 描画モードを通常（不透明）に戻す
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


	// --------------------------------------------------
	// 左側：タイトルとゲーム状態
	// --------------------------------------------------
	DrawStringToHandle(leftX, (int)(screenH * 0.05f), "QUORIDOR", colorWhite, fontTitle_);

	int turnY = (int)(screenH * 0.18f);
	DrawFormatStringToHandle(leftX, turnY, colorGray, fontMain_, "TURN");
	if (currentTurn_ == 0) {
		DrawFormatStringToHandle(leftX, turnY + lineGap, colorRed, fontMain_, "Player 1 (あなた)");
	}
	else {
		DrawFormatStringToHandle(leftX, turnY + lineGap, colorBlue, fontMain_, "Player 2 (CPU)");
	}

	int modeY = (int)(screenH * 0.30f);
	DrawFormatStringToHandle(leftX, modeY, colorGray, fontMain_, "MODE");
	const char* modeText = (mode_ == MODE::MOVE) ? "MOVE [TAB]" : "WALL [TAB]";
	DrawFormatStringToHandle(leftX, modeY + lineGap, colorWhite, fontMain_, "%s", modeText);

	// --------------------------------------------------
	// 左下：操作説明
	// --------------------------------------------------
	int menuY = (int)(screenH * 0.52f);
	int itemGap = (int)(screenH * 0.042f); // 操作説明の行間
	DrawFormatStringToHandle(leftX, menuY, colorGray, fontMain_, "移動   ：方向キー");
	DrawFormatStringToHandle(leftX, menuY + itemGap, colorGray, fontMain_, "モード切替：TAB");
	DrawFormatStringToHandle(leftX, menuY + itemGap * 2, colorGray, fontMain_, "壁の回転 ：RSHIFT");
	DrawFormatStringToHandle(leftX, menuY + itemGap * 3, colorGray, fontMain_, "決定   ：ENTER");
	DrawFormatStringToHandle(leftX, menuY + itemGap * 4, colorGray, fontMain_, "リセット ：R");
	// --------------------------------------------------
	// 右側：プレイヤー情報
	// --------------------------------------------------
	int p1Y = (int)(screenH * 0.18f);
	DrawFormatStringToHandle(rightX, p1Y, colorBlue, fontMain_, "Player 1 (あなた)");
	DrawFormatStringToHandle(rightX, p1Y + lineGap, colorWhite, fontMain_, "残りの壁: %d / %d", players_[0].remainingWalls_, MAX_WALLS);

	int p2Y = (int)(screenH * 0.38f);
	DrawFormatStringToHandle(rightX, p2Y, colorRed, fontMain_, "Player 2 (CPU)");
	DrawFormatStringToHandle(rightX, p2Y + lineGap, colorWhite, fontMain_, "残りの壁: %d / %d", players_[1].remainingWalls_, MAX_WALLS);
}

void Quoridor::Reset(void)
{
	// もし思考中なら結果を一回破棄するなどしてクリアさせる
	if (pythonAI_ && pythonAI_->IsThinking())
	{
		if (pythonAI_->HasResult())
		{
			pythonAI_->TakeResult();
		}
		isCpuThinking_ = false;
	}

	mode_ = MODE::MOVE;

	players_[0] = { 4, 0, 1.0f, 0.31f, 0.31f, 0, 1, 1, 0, MAX_WALLS,{1.0f, 0.31f, 0.31f} };
	players_[1] = { 4, 8, 0.31f, 0.31f, 1.0f, 0, -1, -1, 0, MAX_WALLS, {0.31f, 0.31f, 1.0f} };

	currentTurn_ = 0;
	isChangeTurn_ = false;
	wallCursorX_ = 0;
	wallCursorY_ = 0;
	wallVertical_ = true;
	isGameOver_ = false;
	winner_ = -1;
	isCpuThinking_ = false;

	board_->Init();
	RefreshMoveCandidates();
}

void Quoridor::RefreshMoveCandidates(void)
{
	if (mode_ == MODE::MOVE)
	{
		moveCandidates_ = board_->GetAllMoveCandidates(
			players_[currentTurn_], players_
		);
	}
	else
	{
		moveCandidates_.clear();
	}
}

bool Quoridor::IsBlink(void) const
{
	return ((GetNowCount() / 300) % 2) == 0;
}

void Quoridor::ApplyCpuMove(const std::string& json)
{
	Player& cpu = players_[1];

	DbgLog("[ApplyCpuMove] json=" + json);

	auto findInt = [&](const std::string& key) -> int
		{
			std::string search = "\"" + key + "\": ";
			auto pos = json.find(search);
			if (pos == std::string::npos) return -1;
			size_t valueStart = pos + search.size();
			try { return std::stoi(json.substr(valueStart)); }
			catch (...) { return -1; }
		};

	if (json.find("\"type\": \"move\"") != std::string::npos)
	{
		int x = findInt("x");
		int y = findInt("y");
		DbgLog("[ApplyCpuMove] parsed x=" + std::to_string(x) + " y=" + std::to_string(y));

		if (x >= 0 && y >= 0)
		{
			auto cands = board_->GetAllMoveCandidates(cpu, players_);

			std::string candStr = "[ApplyCpuMove] candidates=";
			for (auto& [cx, cy] : cands)
				candStr += "(" + std::to_string(cx) + "," + std::to_string(cy) + ")";
			DbgLog(candStr);

			bool moved = false;
			for (auto& [cx, cy] : cands)
			{
				if (cx == x && cy == y)
				{
					cpu.x_ = x;
					cpu.y_ = y;
					moved = true;
					DbgLog("[ApplyCpuMove] MOVE APPLIED: cpu -> (" + std::to_string(x) + "," + std::to_string(y) + ")");
					break;
				}
			}
			if (!moved)
			{
				DbgLog("[ApplyCpuMove] REJECTED: (" + std::to_string(x) + "," + std::to_string(y) + ") not in candidates");
			}
		}
		else
		{
			DbgLog("[ApplyCpuMove] invalid x or y (negative)");
		}
	}
	else if (json.find("\"type\": \"wall\"") != std::string::npos)
	{
		if (cpu.remainingWalls_ <= 0) return;

		int x = findInt("x");
		int y = findInt("y");
		bool vertical = (json.find("\"vertical\": true") != std::string::npos);

		if (x >= 0 && y >= 0)
		{
			bool placed = board_->PlaceWall(x, y, vertical, players_, cpu.wallColor_);
			if (placed)
			{
				cpu.remainingWalls_--;
				mode_ = MODE::MOVE;
				DbgLog("[ApplyCpuMove] WALL placed at (" + std::to_string(x) + "," + std::to_string(y) + ")");
			}
		}
	}
	else
	{
		DbgLog("[ApplyCpuMove] unknown type in json");
	}
}

std::string Quoridor::BuildBoardJson(void) const
{
	std::string json = "{";

	json += "\"players\":["
		"[" + std::to_string(players_[0].x_) + "," + std::to_string(players_[0].y_) + "],"
		"[" + std::to_string(players_[1].x_) + "," + std::to_string(players_[1].y_) + "]],";

	json += "\"remaining_walls\":["
		+ std::to_string(players_[0].remainingWalls_) + ","
		+ std::to_string(players_[1].remainingWalls_) + "],";

	auto cands = board_->GetAllMoveCandidates(players_[1], players_);
	json += "\"move_candidates\":[";
	for (int i = 0; i < (int)cands.size(); ++i)
	{
		if (i > 0) json += ",";
		json += "[" + std::to_string(cands[i].first) + "," + std::to_string(cands[i].second) + "]";
	}
	json += "],";

	json += "\"vertical_walls\":[";
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		if (y > 0) json += ",";
		json += "[";
		for (int x = 0; x < BOARD_SIZE - 1; ++x)
		{
			if (x > 0) json += ",";
			json += board_->GetVerticalWall(x, y) ? "1" : "0";
		}
		json += "]";
	}
	json += "],";

	json += "\"horizontal_walls\":[";
	for (int y = 0; y < BOARD_SIZE - 1; ++y)
	{
		if (y > 0) json += ",";
		json += "[";
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			if (x > 0) json += ",";
			json += board_->GetHorizontalWall(x, y) ? "1" : "0";
		}
		json += "]";
	}
	json += "],";

	json += "\"turn\":1}";
	return json;
}

void Quoridor::UpdatePlayer(void)
{
	Player& player = players_[currentTurn_];
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgUp(KEY_INPUT_TAB))
	{
		mode_ = (mode_ == MODE::MOVE) ? MODE::WALL : MODE::MOVE;
		WaitTimer(150);
		RefreshMoveCandidates();
	}

	if (mode_ == MODE::MOVE)
	{
		int DirX = 0, DirY = 0;

		if (ins.IsTrgUp(KEY_INPUT_UP))    DirY += player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_DOWN))  DirY -= player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_LEFT))  DirX -= player.rightDirX_;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT)) DirX += player.rightDirX_;

		if (DirX != 0 || DirY != 0)
		{
			auto cands = board_->GetMoveCandidates(
				player.x_, player.y_, DirX, DirY, players_
			);
			if (!cands.empty())
			{
				player.x_ = cands[0].first;
				player.y_ = cands[0].second;
				isChangeTurn_ = true;
				DbgLog("[UpdatePlayer] moved to (" + std::to_string(player.x_) + "," + std::to_string(player.y_) + ")");
			}
		}
	}
	else if (mode_ == MODE::WALL)
	{
		if (ins.IsTrgUp(KEY_INPUT_UP))    wallCursorY_ += player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_DOWN))  wallCursorY_ -= player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_LEFT))  wallCursorX_ -= player.rightDirX_;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT)) wallCursorX_ += player.rightDirX_;

		if (ins.IsTrgUp(KEY_INPUT_RSHIFT))
			wallVertical_ = !wallVertical_;

		wallCursorX_ = max(0, min(wallCursorX_, BOARD_SIZE - 2));
		wallCursorY_ = max(0, min(wallCursorY_, BOARD_SIZE - 2));

		previewWall_->SetType(wallVertical_ ? Wall::TYPE::VERTICAL : Wall::TYPE::HORIZONTAL);
		previewWall_->SetBoardPosition(wallCursorX_, wallCursorY_);
		previewWall_->RefreshTransform();

		if (ins.IsTrgUp(KEY_INPUT_RETURN))
		{
			if (player.remainingWalls_ <= 0) return;

			bool placed = board_->PlaceWall(
				wallCursorX_, wallCursorY_, wallVertical_, players_, player.wallColor_
			);
			if (placed)
			{
				player.remainingWalls_--;
				mode_ = MODE::MOVE;
				isChangeTurn_ = true;
				WaitTimer(150);
			}
		}
	}
}

void Quoridor::UpdateCpu(void)
{
	if (!pythonAI_ || !pythonAI_->IsRunning())
	{
		DbgLog("[UpdateCpu] PythonAI not running");
		return;
	}

	if (pythonAI_->HasResult())
	{
		DbgLog("[UpdateCpu] HasResult=true");
		std::string response = pythonAI_->TakeResult();
		DbgLog("[UpdateCpu] response=" + response);
		if (!response.empty()) ApplyCpuMove(response);
		isChangeTurn_ = true;
		return;
	}

	if (!pythonAI_->IsThinking())
	{
		std::string boardJson = BuildBoardJson();
		DbgLog("[UpdateCpu] QueryAsync: " + boardJson);
		pythonAI_->QueryAsync(boardJson, nullptr);
	}
}

VECTOR Quoridor::GetWorldPos(int x, int y) const
{
	return VGet(x * Board::CELL_SIZE, 0.0f, y * Board::CELL_SIZE);
}

VECTOR Quoridor::GetCellCenter(int x, int y) const
{
	return VGet(x * Board::CELL_SIZE, 0.0f, y * Board::CELL_SIZE);
}

void Quoridor::DrawBoard(void)
{
	for (int y = 0; y < BOARD_SIZE; y++)
	{
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			VECTOR pos = GetWorldPos(x, y);

			unsigned int cellColor = GetColor(200, 200, 200);
			if (y == BOARD_SIZE - 1) cellColor = GetColor(255, 160, 160);
			if (y == 0)              cellColor = GetColor(160, 160, 255);

			DrawBox3D(
				VAdd(pos, VGet(-4, 0, -4)),
				VAdd(pos, VGet(4, 1, 4)),
				cellColor, TRUE
			);
			DrawBox3D(
				VAdd(pos, VGet(-4, 0, -4)),
				VAdd(pos, VGet(4, 1, 4)),
				GetColor(255, 255, 255), FALSE
			);
		}
	}
}

void Quoridor::DrawMoveCandidates(void)
{
	if (mode_ != MODE::MOVE) return;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);

	for (auto& [cx, cy] : moveCandidates_)
	{
		VECTOR pos = GetWorldPos(cx, cy);
		DrawBox3D(
			VAdd(pos, VGet(-4, 1, -4)),
			VAdd(pos, VGet(4, 4, 4)),
			GetColor(255, 255, 0), TRUE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Quoridor::DrawPlayers(void)
{
	for (int i = 0; i < 2; i++)
	{
		if (i == currentTurn_ && !IsBlink() && mode_ == MODE::MOVE)
		{
			continue;
		}

		playerPieces_[i]->SetBoardPosition(players_[i].x_, players_[i].y_);
		playerPieces_[i]->SetColor(players_[i].r_, players_[i].g_, players_[i].b_);
		playerPieces_[i]->Draw();
	}
}

void Quoridor::DrawWallAndGrid(void)
{
	board_->Draw();
}

void Quoridor::DrawGameOver(void)
{
	int cx = 1280 / 2;
	int cy = 720 / 2;

	unsigned int winColor =
		(winner_ == 0) ? GetColor(255, 80, 80) : GetColor(80, 80, 255);

	DrawFormatString(cx - 80, cy - 20, winColor,
		"Player %d Win!", winner_ + 1);
	DrawFormatString(cx - 80, cy + 10, GetColor(200, 200, 200),
		"Press R to Reset");
}

VECTOR Quoridor::MakeMin(VECTOR a, VECTOR b)
{
	return VGet(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

VECTOR Quoridor::MakeMax(VECTOR a, VECTOR b)
{
	return VGet(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

void Quoridor::DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{
	VECTOR vertexs[8] =
	{
		{min.x,min.y,min.z},
		{max.x,min.y,min.z},
		{max.x,max.y,min.z},
		{min.x,max.y,min.z},
		{min.x,min.y,max.z},
		{max.x,min.y,max.z},
		{max.x,max.y,max.z},
		{min.x,max.y,max.z},
	};

	auto drawFace = [&](int a, int b, int c, int d)
		{
			DrawTriangle3D(vertexs[a], vertexs[b], vertexs[c], color, fillFlag);
			DrawTriangle3D(vertexs[a], vertexs[c], vertexs[d], color, fillFlag);
		};

	drawFace(0, 1, 2, 3);
	drawFace(4, 5, 6, 7);
	drawFace(0, 3, 7, 4);
	drawFace(1, 2, 6, 5);
	drawFace(3, 2, 6, 7);
	drawFace(0, 1, 5, 4);
}

void Quoridor::DrawCube3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{
	VECTOR v[8] =
	{
		VGet(min.x, min.y, min.z),
		VGet(max.x, min.y, min.z),
		VGet(max.x, max.y, min.z),
		VGet(min.x, max.y, min.z),
		VGet(min.x, min.y, max.z),
		VGet(max.x, min.y, max.z),
		VGet(max.x, max.y, max.z),
		VGet(min.x, max.y, max.z),
	};

	auto DrawQuad = [&](int a, int b, int c, int d)
		{
			DrawTriangle3D(v[a], v[b], v[c], color, fillFlag);
			DrawTriangle3D(v[a], v[c], v[d], color, fillFlag);
		};

	DrawQuad(0, 1, 2, 3);
	DrawQuad(5, 4, 7, 6);
	DrawQuad(4, 0, 3, 7);
	DrawQuad(1, 5, 6, 2);
	DrawQuad(3, 2, 6, 7);
	DrawQuad(4, 5, 1, 0);
}