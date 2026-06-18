#include <queue>
#include <fstream>
#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Manager/PythonRuntimeManager.h"
#include "../../Object/Actor/Quoridor/Desk.h"
#include "../../Object/Actor/Quoridor/QuoridorBoard.h"
#include "../../Object/Actor/Quoridor/Wall.h"
#include "../../Object/Actor/Quoridor/PlayerPiece.h"
#include "../../Object/Actor/Quoridor/PythonAI.h"
#include "../../Object/Actor/Quoridor/Triangle.h"
#include "Quoridor.h"

#ifdef _DEBUG
#include <filesystem>
namespace fs = std::filesystem;
#endif // _DEBUG

std::string Quoridor::debugLogPath_ = "quoridor_debug.log";

static void DbgLog(const std::string& msg)
{
	// 空文字の場合はログ出力をスキップ（出力を無効化したいときにも便利です）
	if (Quoridor::GetDebugLogPath().empty()) return;

	std::ofstream f(Quoridor::GetDebugLogPath(), std::ios::app);
	if (f.is_open())
	{
		f << msg << "\n";
	}
}

// 3. 静的関数の実装
void Quoridor::SetDebugLogPath(const std::string& path)
{
	debugLogPath_ = path;
}

std::string Quoridor::GetDebugLogPath()
{
	return debugLogPath_;
}

Quoridor::Quoridor(void)
	:
	gameMode_(GAME_MODE::PLAYER),
	mode_(MODE::MOVE),
	players_{},
	currentTurn_(0),
	isChangeTurn_(false),
	wallCursorX_(0),
	wallCursorY_(0),
	wallVertical_(true),
	isGameOver_(false),
	winner_(-1),
	moveCursorIndex_(0),
	isCpuThinking_(false),
	cpuStartTime_(0),
	fontTitle_(-1),
	fontMain_(-1),
	rFrameCount_(0),
	isWallWarningActive_(false),
	wallWarningTimer_(0),
	isDiagonalSelect_(false)
{
	// プレイヤーの初期化
	players_[0] = { 4, 0, 1.0f, 0.31f, 0.31f, 0, 1, 1, 0, MAX_WALLS,{1.0f, 0.31f, 0.31f} };
	players_[1] = { 4, 8, 0.31f, 0.31f, 1.0f, 0, -1, -1, 0, MAX_WALLS, {0.31f, 0.31f, 1.0f} };

#ifdef _DEBUG
	DbgLog("=== Quoridor START ===");
#endif

}

Quoridor::~Quoridor(void)
{
}

void Quoridor::Init(void)
{
	SetDebugLogPath("Python\\ai\\quoridor_debug.log");

	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::QUORIDOR);

	pMH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_PIECEMOVE_SE).handleId_;
	wMH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_WALLMOVE_SE).handleId_;
	wRH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_WALLROTATION_SE).handleId_;
	mCH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_MODECHANGE_SE).handleId_;
	vicH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_VICTORY_SE).handleId_;
	defH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_PIECEMOVE_SE).handleId_;

	// 描画環境設定
	SetGlobalAmbientLight(GetColorF(0.55f, 0.55f, 0.55f, 1.0f));

	int lightHandle = CreateDirLightHandle(VNorm(VGet(-1.0f, -1.0f, 0.5f)));
	SetLightDifColorHandle(lightHandle, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));

	//SetUseShadowMap(TRUE);
	SetLightEnable(TRUE);

	isReturn_ = false;

	// ゲームモードの設定
	gameMode_ = GAME_MODE::CPU;

	// フォント
	int screenW, screenH;
	GetWindowSize(&screenW, &screenH);

	fontTitle_ = CreateFontToHandle("游明朝", (int)(30.0f * ((float)screenH / 720.0f)), -1, DX_FONTTYPE_ANTIALIASING);
	fontMain_ = CreateFontToHandle("游明朝", (int)(16.0f * ((float)screenH / 720.0f)), -1, DX_FONTTYPE_ANTIALIASING);

	// オブジェクトの初期化
	desk_ = std::make_unique<Desk>();
	desk_->Init();
	desk_->SetPosition(Desk::DEFAULT_POSITION);

	board_ = std::make_unique<QuoridorBoard>();
	board_->Init();

	previewWall_ = std::make_unique<Wall>();
	previewWall_->SetCellSize(QuoridorBoard::CELL_SIZE);
	previewWall_->InitTransform();
	previewWall_->SetType(Wall::TYPE::VERTICAL);

	for (int i = 0; i < 2; ++i)
	{
		playerPieces_[i] = std::make_unique<PlayerPiece>();
		playerPieces_[i]->SetCellSize(QuoridorBoard::CELL_SIZE);
		playerPieces_[i]->Init();
		playerPieces_[i]->SetBoardPosition(players_[i].x_, players_[i].y_);
		playerPieces_[i]->SetColor(players_[i].r_, players_[i].g_, players_[i].b_);
	}

	for (int i = 0; i < moveTriangleIndicators_.size(); ++i)
	{
		moveTriangleIndicators_[i] = std::make_unique<Triangle>();
		moveTriangleIndicators_[i]->Init();
	}

	RefreshMoveCandidates();

	auto& pythonRuntime = PythonRuntimeManager::GetInstance();
	pythonAI_ = std::make_unique<PythonAI>();

#ifdef _DEBUG
	fs::exists(pythonRuntime.GetPythonExePath()) ?
		DbgLog("[Init] Python runtime found: ") :
		DbgLog("[Init] Python runtime NOT found: ");

	if(fs::exists(pythonRuntime.GetPythonExePath()))
	{
		MessageBoxW(
			nullptr,
			pythonRuntime.GetPythonExePath().c_str(),
			L"Path",
			MB_OK
		);
	}
	if (fs::exists(pythonRuntime.GetPythonExePath()))
	{
		MessageBoxW(nullptr, L"FOUND", L"DEBUG", MB_OK);
	}
	else
	{
		MessageBoxW(nullptr, L"NOT FOUND", L"DEBUG", MB_OK);
	}

#endif // DEBUG

	// Python接続開始
	bool ok = pythonAI_->Start(
		pythonRuntime.GetPythonExePath(),
		pythonRuntime.GetScriptPath(L"CPU_AI\\Quoridor_Ai.py")
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
		rFrameCount_++;

		if (!isReturn_ &&
			rFrameCount_ > 180)
		{
			isReturn_ = true;
			return;
		}

		if (ins.IsTrgUp(KEY_INPUT_R))
		{
#ifdef _DEBUG
			DbgLog("[Update] R key pressed. Restarting game!");
			Reset();
#endif
			return;
		}
	}

	if (isGameOver_) return;

	// 警告タイマーのカウントダウン更新
	if (isWallWarningActive_)
	{
		wallWarningTimer_--;
		if (wallWarningTimer_ <= 0)
		{
			isWallWarningActive_ = false;
		}
	}

	desk_->Update();

	for (int i = 0; i < moveTriangleIndicators_.size(); ++i)
	{
		moveTriangleIndicators_[i]->Update();
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
		if (!isGameOver_)
		{
			isGameOver_ = true;

			if (winner_ == 0)
			{
				PlaySoundMem(vicH_, DX_PLAYTYPE_BACK);
			}
			else
			{
				PlaySoundMem(defH_, DX_PLAYTYPE_BACK);
			}
		}
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
	// 1. 現在の画面サイズを動的に取得
	int screenW, screenH;
	GetWindowSize(&screenW, &screenH);

	// 2. 画面サイズ（特に縦幅）の変化を検知してフォントサイズを動的更新
	// 初回に Init で作ったフォントを消さないよう、初期値を screenH に合わせておく
	static int lastScreenH = screenH;
	if (screenH != lastScreenH)
	{
		if (fontTitle_ != -1) DeleteFontToHandle(fontTitle_);
		if (fontMain_ != -1)  DeleteFontToHandle(fontMain_);

		int titleFontSize = (int)(32.0f * ((float)screenH / 720.0f));
		int mainFontSize = (int)(12.0f * ((float)screenH / 720.0f));

		if (titleFontSize < 16) titleFontSize = 16;
		if (mainFontSize < 9)   mainFontSize = 9;

		// 環境依存の少ない DX_FONTTYPE_ANTIALIAS 変更して安全性を高める
		fontTitle_ = CreateFontToHandle("游明朝", titleFontSize, 3, DX_FONTTYPE_ANTIALIASING);
		fontMain_ = CreateFontToHandle("游明朝", mainFontSize, 2, DX_FONTTYPE_ANTIALIASING);

		lastScreenH = screenH;
	}

	// 各種色の定義
	unsigned int colorWhite = GetColor(240, 230, 220);
	unsigned int colorGray = GetColor(150, 140, 130);
	unsigned int colorRed = GetColor(200, 80, 80);
	unsigned int colorBlue = GetColor(80, 80, 200);

	// 3. 座標を画面サイズの「割合（％）」で動的に計算
	int leftX = (int)(screenW * 0.04f);
	int rightX = (int)(screenW * 0.78f);
	int lineGap = (int)(screenH * 0.04f);

	// --- 背景ボックスの描画 ---
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
	// 左側のUI背景ボックス
	DrawBox(
		(int)(screenW * 0.02f), (int)(screenH * 0.03f),
		(int)(screenW * 0.25f), (int)(screenH * 0.78f),
		GetColor(25, 20, 15), TRUE
	);
	// 右側のUI背景ボックス
	DrawBox(
		(int)(screenW * 0.76f), (int)(screenH * 0.14f),
		(int)(screenW * 0.98f), (int)(screenH * 0.52f),
		GetColor(25, 20, 15), TRUE
	);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// --------------------------------------------------
	// 左側：タイトルとゲーム状態
	// --------------------------------------------------
	DrawStringToHandle(leftX - static_cast<int>(leftX * 0.5f), (int)(screenH * 0.05f), "QUORIDOR", colorWhite, fontTitle_);

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
	const char* modeText = (mode_ == MODE::MOVE) ? "MOVE" : "WALL";
	DrawFormatStringToHandle(leftX, modeY + lineGap, colorWhite, fontMain_, "%s", modeText);

	bool isPadConnected = (GetJoypadNum() > 0);

	// --------------------------------------------------
	// 左下：操作説明
	// --------------------------------------------------
	int menuY = (int)(screenH * 0.52f);
	int itemGap = (int)(screenH * 0.042f);

	if (isPadConnected)
	{
		DrawFormatStringToHandle(leftX, menuY, colorGray, fontMain_, "移動      ：十字ボタン");
		DrawFormatStringToHandle(leftX, menuY + itemGap, colorGray, fontMain_, "モード切替：X");
		DrawFormatStringToHandle(leftX, menuY + itemGap * 2, colorGray, fontMain_, "壁の回転  ：Y");
		DrawFormatStringToHandle(leftX, menuY + itemGap * 3, colorGray, fontMain_, "決定      ：A");
		
		if (isDiagonalSelect_)
		{
			DrawFormatStringToHandle(leftX, menuY + itemGap * 4, colorGray, fontMain_, "斜め移動  ：前後→左右→ENTER");
			DrawFormatStringToHandle(leftX, menuY + itemGap * 5, colorGray, fontMain_, "斜め移動取り消し  ：B");
		}
	}
	else
	{
		DrawFormatStringToHandle(leftX, menuY, colorGray, fontMain_, "移動      ：方向キー");
		DrawFormatStringToHandle(leftX, menuY + itemGap, colorGray, fontMain_, "モード切替：TAB");
		DrawFormatStringToHandle(leftX, menuY + itemGap * 2, colorGray, fontMain_, "壁の回転  ：RSHIFT");
		DrawFormatStringToHandle(leftX, menuY + itemGap * 3, colorGray, fontMain_, "決定      ：ENTER");
		
		if (isDiagonalSelect_)
		{
			DrawFormatStringToHandle(leftX, menuY + itemGap * 4, colorGray, fontMain_, "斜め移動  ：前後→左右→ENTER");
			DrawFormatStringToHandle(leftX, menuY + itemGap * 5, colorGray, fontMain_, "斜め移動取り消し  ：BackSpace");
		}
	}


	// --------------------------------------------------
	// 右側：プレイヤー情報
	// --------------------------------------------------
	int p1Y = (int)(screenH * 0.18f);
	DrawFormatStringToHandle(rightX - static_cast<int>(rightX * 0.009f), p1Y, colorRed, fontMain_, "Player 1 (あなた)");
	DrawFormatStringToHandle(rightX - static_cast<int>(rightX * 0.009f), p1Y + lineGap, colorWhite, fontMain_, "残りの壁: %d / %d", players_[0].remainingWalls_, MAX_WALLS);

	int p2Y = (int)(screenH * 0.38f);
	DrawFormatStringToHandle(rightX - static_cast<int>(rightX * 0.009f), p2Y, colorBlue, fontMain_, "Player 2 (CPU)");
	DrawFormatStringToHandle(rightX - static_cast<int>(rightX * 0.009f), p2Y + lineGap, colorWhite, fontMain_, "残りの壁: %d / %d", players_[1].remainingWalls_, MAX_WALLS);

	// --- ゲームオーバー時はゲームオーバー画面だけを描画して終了する ---
	if (isGameOver_)
	{
		DrawGameOver();
	}

	// --------------------------------------------------
	// 【追加】左側：CPUの「考えてる感」を出すインジケータ演出
	// --------------------------------------------------
	if (gameMode_ == GAME_MODE::CPU && currentTurn_ == 1 && isCpuThinking_)
	{
		// 300ミリ秒ごとに「.」「..」「...」「」と文字が変化する
		int dotCount = (GetNowCount() / 300) % 4;
		std::string thinkingText = "CPU思考中";
		for (int i = 0; i < dotCount; ++i) {
			thinkingText += ".";
		}
		// モードテキストのさらに下に、少し目立つオレンジ〜黄色系の色で描画
		DrawFormatStringToHandle(leftX, static_cast<int>(modeY + (lineGap * 2.5f)), GetColor(240, 200, 80), fontMain_, "%s", thinkingText.c_str());
	}

	// --------------------------------------------------
	// 【注目】壁がないときの警告テロップ描画
	// --------------------------------------------------
	if (isWallWarningActive_)
	{
		int boxW = (int)(screenW * 0.45f);
		int boxH = (int)(screenH * 0.07f);
		int boxX = (screenW - boxW) / 2;
		int boxY = (int)(screenH * 0.88f);

		// 1. 背景ボックス
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
		DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(80, 20, 20), TRUE); // 警告なので少し赤を強めに
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 2. 文字
		const char* warnText = "残りの壁が 0 なので配置できません！";

		int textW = GetDrawStringWidthToHandle(warnText, (int)strlen(warnText), fontMain_);
		int textX = boxX + (boxW - textW) / 2;
		int textY = boxY + (boxH - (int)(18.0f * ((float)screenH / 720.0f))) / 2;

		unsigned int colorWarn = GetColor(255, 120, 100);

		// ⚠️ここを DrawStringToHandle からフォーマット対応の DrawFormatStringToHandle に統一
		DrawFormatStringToHandle(textX, textY, colorWarn, fontMain_, "%s", warnText);
	}
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
	cpuStartTime_ = 0;

	isReturn_ = false;

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
					PlaySoundMem(pMH_, DX_PLAYTYPE_BACK);
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
				PlaySoundMem(wMH_, DX_PLAYTYPE_BACK);
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

	//static bool isDiagChoosing = false; // 斜め移動の候補選択中かどうか
	//static std::pair<int, int> diagTarget = { 0,0 }; // 選択中の斜め移動先座標

	if (ins.IsTrgUp(KEY_INPUT_TAB)||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1,InputManager::JOYPAD_BTN::LEFT))
	{
		if (isDiagChoosing_)
		{
			isDiagChoosing_ = false;
			isDiagonalSelect_ = false;
			return;
		}

		// 移動モードから壁モードに切り替える前に、壁が残っているかチェック
		if (mode_ == MODE::MOVE && player.remainingWalls_ <= 0)
		{
			// 壁モードで壁が残ってない場合は切り替え禁止＆警告表示
			isWallWarningActive_ = true;
			wallWarningTimer_ = 120; // 約2秒間警告表示
			mode_ = MODE::MOVE; // 強制的に移動モードに切り替え
			WaitTimer(150);
			return;
		}

		if (player.remainingWalls_ > 0)
		{
			mode_ = (mode_ == MODE::MOVE) ? MODE::WALL : MODE::MOVE;
			PlaySoundMem(mCH_, DX_PLAYTYPE_BACK);
			WaitTimer(150);
			RefreshMoveCandidates();
		}
	}

	if (mode_ == MODE::MOVE)
	{
		if (isDiagChoosing_)
		{
			if (ins.IsTrgUp(KEY_INPUT_RETURN)||
				ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
			{
				player.x_ = diagTarget_.first;
				player.y_ = diagTarget_.second;
				PlaySoundMem(pMH_, DX_PLAYTYPE_BACK);
				isChangeTurn_ = true;
				isDiagChoosing_ = false; // 状態を解除
				isDiagonalSelect_ = false;
				DbgLog("[UpdatePlayer] 斜め移動確定(ENTER): (" + std::to_string(player.x_) + "," + std::to_string(player.y_) + ")");
				WaitTimer(150);
				return;
			}

			if (ins.IsTrgUp(KEY_INPUT_BACK)||
				ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT))
			{
				isDiagChoosing_ = false;
				isDiagonalSelect_ = false;
				DbgLog("[UpdatePlayer] 斜め移動キャンセル(BACK)");
				WaitTimer(150);
				return;
			}

			return;
		}

		int DirX = 0, DirY = 0;

		if (ins.IsTrgUp(KEY_INPUT_UP)|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP))    DirY += player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_DOWN)|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN))  DirY -= player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_LEFT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_LEFT))  DirX -= player.rightDirX_;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_RIGHT)) DirX += player.rightDirX_;

		if (DirX != 0 || DirY != 0)
		{
			auto cands = board_->GetMoveCandidates(
				player.x_, player.y_, DirX, DirY, players_
			);
			if (!cands.empty())
{
				// 取得した移動先（cands[0]）が、現在の位置から見て「斜め」かどうかをチェック
				// （X座標もY座標も両方とも変わっていれば斜め移動）
				bool isDiagonal = (cands[0].first != player.x_) && (cands[0].second != player.y_);

				if (!isDiagonal)
				{
					// ─── 【通常移動】 ───
					// 十字方向への移動（または敵を直線に飛び越える移動）なら即座に確定
					player.x_ = cands[0].first;
					player.y_ = cands[0].second;
					PlaySoundMem(pMH_, DX_PLAYTYPE_BACK);
					isChangeTurn_ = true;
					DbgLog("[UpdatePlayer] 通常移動確定: (" + std::to_string(player.x_) + "," + std::to_string(player.y_) + ")");
				}
				else
				{
					// 【斜めジャンプ】 選択モードへ移行
					isDiagChoosing_ = true;
					isDiagonalSelect_ = true;
					diagTarget_ = cands[0]; // 最初に見つかった斜め移動先をターゲットに設定

					DbgLog("[UpdatePlayer] 斜めジャンプ選択モード開始。確定待ちターゲット: (" + std::to_string(diagTarget_.first) + "," + std::to_string(diagTarget_.second) + ")");
				}
			}
		}
	}
	else if (mode_ == MODE::WALL)
	{
		bool moved = false;

		if (ins.IsTrgUp(KEY_INPUT_UP) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_UP)) { wallCursorY_ += player.forwardDirY_; moved = true; }
		if (ins.IsTrgUp(KEY_INPUT_DOWN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_DOWN)) { wallCursorY_ -= player.forwardDirY_; moved = true; }
		if (ins.IsTrgUp(KEY_INPUT_LEFT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_LEFT)) { wallCursorX_ -= player.rightDirX_; moved = true;}
		if (ins.IsTrgUp(KEY_INPUT_RIGHT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DPAD_RIGHT)) { wallCursorX_ += player.rightDirX_; moved = true;	}

		if (moved)
		{
			PlaySoundMem(wMH_, DX_PLAYTYPE_BACK);
		}

		if (ins.IsTrgUp(KEY_INPUT_RSHIFT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP))
		{
			wallVertical_ = !wallVertical_;
			PlaySoundMem(wRH_, DX_PLAYTYPE_BACK);
		}

		wallCursorX_ = max(0, min(wallCursorX_, BOARD_SIZE - 2));
		wallCursorY_ = max(0, min(wallCursorY_, BOARD_SIZE - 2));

		previewWall_->SetType(wallVertical_ ? Wall::TYPE::VERTICAL : Wall::TYPE::HORIZONTAL);
		previewWall_->SetBoardPosition(wallCursorX_, wallCursorY_);
		previewWall_->RefreshTransform();

		if (ins.IsTrgUp(KEY_INPUT_RETURN) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
		{
			if (player.remainingWalls_ <= 0)return;

			bool placed = board_->PlaceWall(
				wallCursorX_, wallCursorY_, wallVertical_, players_, player.wallColor_
			);
			if (placed)
			{
				player.remainingWalls_--;
				PlaySoundMem(wMH_, DX_PLAYTYPE_BACK);
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

	// ─── 【改善】すでに結果が届いている場合の処理 ───
	if (pythonAI_->HasResult())
	{
		// 演出用の最低思考時間（ミリ秒）。400ms ほど「考えたフリ」をさせる
		const int MIN_THINKING_TIME_MS = 400;
		int elapsedTime = GetNowCount() - cpuStartTime_;

		// 最低思考時間を過ぎるまでは、画面をフリーズさせずに「考え中」のまま待機させる
		if (elapsedTime < MIN_THINKING_TIME_MS)
		{
			return;
		}

		DbgLog("[UpdateCpu] HasResult=true & MinTime passed");
		std::string response = pythonAI_->TakeResult();
		DbgLog("[UpdateCpu] response=" + response);

		if (!response.empty()) ApplyCpuMove(response);

		// 思考終了
		isCpuThinking_ = false;
		isChangeTurn_ = true;
		return;
	}

	// ─── 【改善】まだPythonにリクエストを送っていない（思考開始時） ───
	if (!pythonAI_->IsThinking())
	{
		std::string boardJson = BuildBoardJson();

		// 【アイデア3の拡張】CPU（players_[1]）の壁が0枚なら、JSONの末尾に拡張ヒントを仕込む
		if (players_[1].remainingWalls_ <= 0)
		{
			// JSONの閉じ括弧「}」を削って、推奨する探索深さ(1)を結合して閉じ直す
			if (!boardJson.empty() && boardJson.back() == '}') {
				boardJson.pop_back();
				boardJson += ",\"suggested_depth\":1}";
			}
		}

		DbgLog("[UpdateCpu] QueryAsync: " + boardJson);

		// 非同期リクエスト開始
		pythonAI_->QueryAsync(boardJson, nullptr);

		// 【演出用】考え始めた時間をミリ秒で記録し、思考中フラグをON
		cpuStartTime_ = GetNowCount();
		isCpuThinking_ = true;
	}
}

VECTOR Quoridor::GetWorldPos(int x, int y) const
{
	return VGet(x * QuoridorBoard::CELL_SIZE, 0.0f, y * QuoridorBoard::CELL_SIZE);
}

VECTOR Quoridor::GetCellCenter(int x, int y) const
{
	return VGet(x * QuoridorBoard::CELL_SIZE, 0.0f, y * QuoridorBoard::CELL_SIZE);
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
	// 現在のターン主の情報を取得
	const Player& curPlayer = players_[currentTurn_];

	// 選択中のコマの現在の世界座標を取得
	VECTOR playerPos = GetWorldPos(curPlayer.x_, curPlayer.y_);

	// 斜め移動の候補がある場合、通常のオフセットより少し広めに取るための変数
	float offset = QuoridorBoard::CELL_SIZE * 0.58f;
	float diagOffset = QuoridorBoard::CELL_SIZE * 1.0f;

	// 8方向の設定定義（0~3: 十字、4~7: 斜め）
	struct DirectionConfig {
		int dx;
		int dy;
		float offsetX;
		float offsetZ;
		VECTOR rotation;
		bool isDiagonal; // 斜め判定フラグ
	};

	DirectionConfig dirs[8] = {
		{  0,  1,   0.0f,   offset, VGet(0.0f, 0.0f, 0.0f),					false },  // 0: 上
		{  0, -1,   0.0f,  -offset, VGet(0.0f, DX_PI_F, 0.0f),				false },  // 1: 下
		{ -1,  0,  -offset,   0.0f, VGet(0.0f, -DX_PI_F * 0.5f, 0.0f),		false },  // 2: 左
		{  1,  0,   offset,   0.0f, VGet(0.0f,  DX_PI_F * 0.5f, 0.0f),		false },  // 3: 右

		{ -1,  1, -diagOffset,  diagOffset, VGet(0.0f, -DX_PI_F * 0.25f, 0.0f), true },  // 4: 左上
		{  1,  1,  diagOffset,  diagOffset, VGet(0.0f,  DX_PI_F * 0.25f, 0.0f),  true },  // 5: 右上
		{ -1, -1, -diagOffset, -diagOffset, VGet(0.0f, -DX_PI_F * 0.75f, 0.0f), true },  // 6: 左下
		{  1, -1,  diagOffset, -diagOffset, VGet(0.0f,  DX_PI_F * 0.75f, 0.0f),  true }   // 7: 右下	
	};

	for (int i = 0; i < 8; ++i)
	{
		// 斜め移動にしようとしている（Enter待ち）のときは、十字方向の矢印（0〜3）を非表示にする
		if (isDiagChoosing_ && !dirs[i].isDiagonal)
		{
			continue;
		}

		// その方向の座標を計算
		int targetX = curPlayer.x_ + dirs[i].dx;
		int targetY = curPlayer.y_ + dirs[i].dy;

		// 移動候補リストの中に、この方向の座標が含まれているかを「全探索」して調べる
		bool isAvailable = false;
		for (auto& [cx, cy] : moveCandidates_)
		{
			if (cx == targetX && cy == targetY)
			{
				isAvailable = true;
				break;
			}
		}

		// 斜め選択中（Enter待ち）のときは、今選択していない方の斜め矢印は消す
		if (isDiagChoosing_ && dirs[i].isDiagonal)
		{
			if (targetX != diagTarget_.first || targetY != diagTarget_.second)
			{
				continue; // 選択中のターゲット以外の斜め矢印は描画しない
			}
		}

		// 描画処理
		if (isAvailable && moveTriangleIndicators_[i])
		{
			// 十字と斜めで色（表現）を差別化する
			if (dirs[i].isDiagonal)
			{
				if (isDiagChoosing_ && IsBlink())
				{
					moveTriangleIndicators_[i]->SetColor(1.0f, 1.0f, 0.0f); // 鮮やかな黄色
				}
				else
				{
					moveTriangleIndicators_[i]->SetColor(0.8f, 0.7f, 0.2f); // 落ち着いたゴールド
				}
			}
			else
			{
				// 十字方向の矢印：通常のプレイヤーカラー
				moveTriangleIndicators_[i]->SetColor(curPlayer.r_, curPlayer.g_, curPlayer.b_);
			}

			// トランスフォーム適用
			float posX = playerPos.x + dirs[i].offsetX;
			float posZ = playerPos.z + dirs[i].offsetZ;
			moveTriangleIndicators_[i]->SetPositon(posX, posZ);
			moveTriangleIndicators_[i]->SetRotation(dirs[i].rotation);

			// 描画
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
			moveTriangleIndicators_[i]->Draw();
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
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
	// 1. 現在の画面サイズを動的に取得
	int screenW, screenH;
	GetWindowSize(&screenW, &screenH);

	// 2. 画面全体のトーンを落とす（半透明の黒で全画面を覆う）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, screenW, screenH, GetColor(10, 10, 10), TRUE);

	// 3. 中央に文字を際立たせるための「焦げ茶色の帯」を描画
	int bandH = (int)(screenH * 0.22f);           // 画面の高さの22%の太さ
	int bandY = (screenH - bandH) / 2;            // 画面の垂直中央
	DrawBox(0, bandY, screenW, bandY + bandH, GetColor(35, 25, 20), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 4. 勝者に応じた演出色の決定
	// 目標画面のイメージに合わせ、上品なゴールド寄りの発色にします
	unsigned int colorWinText = GetColor(245, 220, 180);
	if (winner_ == 0) {
		// プレイヤー1（あなた）が勝った場合
		colorWinText = GetColor(255, 150, 150); // 落ち着いたライトレッド
	}
	else if (winner_ == 1) {
		// プレイヤー2（CPU）が勝った場合
		colorWinText = GetColor(150, 200, 255); // さわやかなライトブルー
	}

	// 5. 勝者メッセージのテキスト作成と中央揃え描画
	char winText[64];
	if (winner_ == 0) {
		sprintf_s(winText, "PLAYER 1 (YOU) WINS!");
	}
	else {
		sprintf_s(winText, "PLAYER 2 (CPU) WINS!");
	}

	// 大きなフォント（fontTitle_）を使って中央に配置
	int textW = GetDrawStringWidthToHandle(winText, (int)strlen(winText), fontTitle_);
	int textX = (screenW - textW) / 2;
	int textY = bandY + (bandH - (int)(40.0f * ((float)screenH / 720.0f))) / 2 - 10; // 帯のやや上寄り
	DrawFormatStringToHandle(textX, textY, colorWinText, fontTitle_, "%s", winText);

	char bottomText[64] = "";

#ifdef _DEBUG
	// 【デバッグモード】Rキーで即時リセットできる案内
	sprintf_s(bottomText, "- Press [ R ] to Restart Game -");
	unsigned int textColor = GetColor(170, 160, 150); // デバッグ用グレー
#else
	// 【リリースモード】時間経過で終了する案内（残り秒数を動的に計算）
	int remainingFrames = 180 - rFrameCount_;
	if (remainingFrames < 0) remainingFrames = 0;

	// 60FPS想定で秒数に変換（少数を切り上げて綺麗に見せる）
	int remainingSeconds = (remainingFrames + 59) / 60;

	sprintf_s(bottomText, "Returning to menu in %d seconds...", remainingSeconds);
	unsigned int textColor = GetColor(150, 160, 150); // リリース用、少し落ち着いたセージ色
#endif

	// 計算したテキストを画面中央下部に描画
	int bottomW = GetDrawStringWidthToHandle(bottomText, (int)strlen(bottomText), fontMain_);
	int bottomX = (screenW - bottomW) / 2;
	int bottomY = bandY + bandH - (int)(30.0f * ((float)screenH / 720.0f)); // 帯の下寄り

	// 修正された変数（textColor）を使って文字を描画
	DrawFormatStringToHandle(bottomX, bottomY, textColor, fontMain_, "%s", bottomText);
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