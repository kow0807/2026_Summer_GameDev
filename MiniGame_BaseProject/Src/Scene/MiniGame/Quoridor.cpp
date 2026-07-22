#include <queue>
#include <fstream>
#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Object/Actor/Quoridor/Desk.h"
#include "../../Object/Actor/Quoridor/QuoridorBoard.h"
#include "../../Object/Actor/Quoridor/Wall.h"
#include "../../Object/Actor/Quoridor/PlayerPiece.h"
#include "../../Object/Actor/Quoridor/PythonAI.h"
#include "../../Object/Actor/Quoridor/Triangle.h"
#include "../../Object/Actor/Quoridor/QuoridorCpu.h"
#include "../../Application.h"
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

	pMSe_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_PIECEMOVE_SE).handleId_;
	wMSe_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_WALLMOVE_SE).handleId_;
	wRSe_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_WALLROTATION_SE).handleId_;
	mCSe_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_MODECHANGE_SE).handleId_;
	vicSe_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_VICTORY_SE).handleId_;
	loseSe_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_LOSE_SE).handleId_;

	menuSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MENU_SE).handleId_;
	cancelSe_ = resMng_.Load(ResourceManager::SRC::SELECT_CANCEL_SE).handleId_;
	moveSe_ = resMng_.Load(ResourceManager::SRC::SELECT_MOVE_SE).handleId_;
	decideSEH_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_DICIDE_SE).handleId_;
	bgm_ = resMng_.Load(ResourceManager::SRC::QUORIDOR_BGM).handleId_;
	ChangeVolumeSoundMem(190, bgm_);
	isBgm_ = true;

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

	cpu_ = std::make_unique<QuoridorCpu>();


}

void Quoridor::Update(void)
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

	auto& ins = InputManager::GetInstance();
	if (isGameOver_)
	{
		rFrameCount_++;

		if (!isReturn_ &&
			rFrameCount_ > 180)
		{
			isReturn_ = true;
			StopSoundMem(bgm_);
			StopSoundMem(loseSe_);
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
				PlaySoundMem(vicSe_, DX_PLAYTYPE_BACK);
			}
			else
			{
				PlaySoundMem(loseSe_, DX_PLAYTYPE_BACK);
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
	int screenW = 0;
	int screenH = 0;
	GetWindowSize(&screenW, &screenH);

	// --------------------------------------------------
	// フォント更新
	// --------------------------------------------------

	if (screenH != lastUiScreenH_)
	{
		if (fontTitle_ != -1)
		{
			DeleteFontToHandle(fontTitle_);
		}

		if (fontMain_ != -1)
		{
			DeleteFontToHandle(fontMain_);
		}

		const float scale = static_cast<float>(screenH) / 720.0f;

		int emphasisFontSize = static_cast<int>(24.0f * scale);
		int mainFontSize = static_cast<int>(16.0f * scale);

		if (emphasisFontSize < 18)
		{
			emphasisFontSize = 18;
		}

		if (mainFontSize < 12)
		{
			mainFontSize = 12;
		}

		// fontTitle_ はタイトルではなく、強調文字用として使用
		fontTitle_ = CreateFontToHandle(
			"BIZ UDPゴシック",
			emphasisFontSize,
			5,
			DX_FONTTYPE_ANTIALIASING
		);

		fontMain_ = CreateFontToHandle(
			"BIZ UDPゴシック",
			mainFontSize,
			3,
			DX_FONTTYPE_ANTIALIASING
		);

		lastUiScreenH_ = screenH;
	}

	if (fontTitle_ == -1 || fontMain_ == -1)
	{
		return;
	}

	// --------------------------------------------------
	// 色
	// --------------------------------------------------
	const unsigned int colorText =
		GetColor(238, 233, 224);

	const unsigned int colorSubText =
		GetColor(172, 166, 156);

	const unsigned int colorPanel =
		GetColor(22, 19, 17);

	const unsigned int colorPanelBorder =
		GetColor(100, 89, 75);

	const unsigned int colorGold =
		GetColor(205, 169, 91);

	const unsigned int colorPlayer =
		GetColor(210, 92, 82);

	const unsigned int colorCpu =
		GetColor(100, 125, 210);

	const unsigned int colorWarning =
		GetColor(232, 105, 83);

	const unsigned int colorEmptyWall =
		GetColor(73, 67, 60);

	// --------------------------------------------------
	// 共通描画ラムダ
	// --------------------------------------------------

	// 半透明パネル
	auto DrawPanel =
		[&](int x1, int y1, int x2, int y2, int alpha = 190)
		{
			// 影
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha / 2);

			DrawBox(
				x1 + 4,
				y1 + 5,
				x2 + 4,
				y2 + 5,
				GetColor(0, 0, 0),
				TRUE
			);

			// 本体
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

			DrawBox(
				x1,
				y1,
				x2,
				y2,
				colorPanel,
				TRUE
			);

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			// 細い枠
			DrawBox(
				x1,
				y1,
				x2,
				y2,
				colorPanelBorder,
				FALSE
			);
		};

	// 中央揃え文字
	auto DrawCenteredText =
		[&](int centerX,
			int y,
			const char* text,
			unsigned int color,
			int fontHandle)
		{
			const int textWidth = GetDrawStringWidthToHandle(
				text,
				static_cast<int>(strlen(text)),
				fontHandle
			);

			DrawStringToHandle(
				centerX - textWidth / 2,
				y,
				text,
				color,
				fontHandle
			);
		};

	// 残り壁ゲージ
	auto DrawWallGauge =
		[&](int x,
			int y,
			int remainingWalls,
			unsigned int activeColor)
		{
			const int iconCount = MAX_WALLS;
			const int iconWidth =
				static_cast<int>(screenW * 0.008f);

			const int iconHeight =
				static_cast<int>(screenH * 0.019f);

			const int iconGap =
				static_cast<int>(screenW * 0.003f);

			for (int i = 0; i < iconCount; ++i)
			{
				const int iconX =
					x + i * (iconWidth + iconGap);

				const unsigned int iconColor =
					(i < remainingWalls)
					? activeColor
					: colorEmptyWall;

				DrawBox(
					iconX,
					y,
					iconX + iconWidth,
					y + iconHeight,
					iconColor,
					TRUE
				);
			}
		};

	// プレイヤーカード
	auto DrawPlayerCard =
		[&](int x,
			int y,
			int width,
			int height,
			const char* playerName,
			int remainingWalls,
			unsigned int playerColor,
			bool isActive)
		{
			DrawPanel(
				x,
				y,
				x + width,
				y + height,
				isActive ? 215 : 170
			);

			// 手番中のアクセントライン
			if (isActive)
			{
				const int accentWidth =
					static_cast<int>(screenW * 0.004f);

				DrawBox(
					x,
					y,
					x + accentWidth,
					y + height,
					colorGold,
					TRUE
				);
			}

			const int paddingX =
				static_cast<int>(screenW * 0.015f);

			const int paddingY =
				static_cast<int>(screenH * 0.018f);

			DrawStringToHandle(
				x + paddingX,
				y + paddingY,
				playerName,
				playerColor,
				fontMain_
			);

			const char* turnText =
				isActive ? "手番" : "";

			if (isActive)
			{
				const int turnTextWidth =
					GetDrawStringWidthToHandle(
						turnText,
						static_cast<int>(strlen(turnText)),
						fontMain_
					);

				DrawStringToHandle(
					x + width - paddingX - turnTextWidth,
					y + paddingY,
					turnText,
					colorGold,
					fontMain_
				);
			}

			const int gaugeY =
				y + static_cast<int>(height * 0.57f);

			DrawWallGauge(
				x + paddingX,
				gaugeY,
				remainingWalls,
				playerColor
			);

			DrawFormatStringToHandle(
				x + width -
				static_cast<int>(screenW * 0.052f),
				gaugeY - 2,
				colorText,
				fontMain_,
				"%d",
				remainingWalls
			);
		};

	// --------------------------------------------------
	// ゲームオーバー
	// --------------------------------------------------
	// 通常HUDの上に描画するのではなく、
	// ゲームオーバー画面を最優先にする
	if (isGameOver_)
	{
		DrawGameOver();
		return;
	}

	// --------------------------------------------------
// 左上：現在のターン
// --------------------------------------------------
	const int turnPanelWidth =
		static_cast<int>(screenW * 0.25f);

	const int turnPanelHeight =
		static_cast<int>(screenH * 0.105f);

	const int turnPanelX =
		static_cast<int>(screenW * 0.03f);

	const int turnPanelY =
		static_cast<int>(screenH * 0.035f);

	DrawPanel(
		turnPanelX,
		turnPanelY,
		turnPanelX + turnPanelWidth,
		turnPanelY + turnPanelHeight,
		180
	);

	const char* turnText =
		(currentTurn_ == 0)
		? "あなたのターン"
		: "CPUのターン";

	const unsigned int turnColor =
		(currentTurn_ == 0)
		? colorPlayer
		: colorCpu;

	const char* modeText =
		(mode_ == MODE::MOVE)
		? "駒を移動"
		: "壁を配置";

	const int textX =
		turnPanelX + static_cast<int>(screenW * 0.018f);

	DrawStringToHandle(
		textX,
		turnPanelY + static_cast<int>(screenH * 0.014f),
		turnText,
		turnColor,
		fontTitle_
	);

	DrawStringToHandle(
		textX,
		turnPanelY + static_cast<int>(screenH * 0.060f),
		modeText,
		colorSubText,
		fontMain_
	);

	// --------------------------------------------------
	// 右上：プレイヤー情報
	// --------------------------------------------------
	const int cardWidth =
		static_cast<int>(screenW * 0.205f);

	const int cardHeight =
		static_cast<int>(screenH * 0.105f);

	const int cardX =
		screenW - cardWidth -
		static_cast<int>(screenW * 0.025f);

	const int cardTop =
		static_cast<int>(screenH * 0.17f);

	const int cardGap =
		static_cast<int>(screenH * 0.018f);

	DrawPlayerCard(
		cardX,
		cardTop,
		cardWidth,
		cardHeight,
		"あなた",
		players_[0].remainingWalls_,
		colorPlayer,
		currentTurn_ == 0
	);

	DrawPlayerCard(
		cardX,
		cardTop + cardHeight + cardGap,
		cardWidth,
		cardHeight,
		"CPU",
		players_[1].remainingWalls_,
		colorCpu,
		currentTurn_ == 1
	);

	// --------------------------------------------------
	// 下中央：状況に応じた操作ガイド
	// --------------------------------------------------
	const bool isPadConnected =
		(GetJoypadNum() > 0);

	const int guideWidth =
		static_cast<int>(screenW * 0.61f);

	const int guideHeight =
		static_cast<int>(screenH * 0.075f);

	const int guideX =
		(screenW - guideWidth) / 2;

	const int guideY =
		screenH - guideHeight -
		static_cast<int>(screenH * 0.025f);

	DrawPanel(
		guideX,
		guideY,
		guideX + guideWidth,
		guideY + guideHeight,
		165
	);

	std::string guideText;

	if (isDiagonalSelect_)
	{
		if (isPadConnected)
		{
			guideText =
				"十字ボタン：方向選択    A：決定    B：取り消し";
		}
		else
		{
			guideText =
				"方向キー：方向選択    ENTER：決定    BACKSPACE：取り消し";
		}
	}
	else if (mode_ == MODE::MOVE)
	{
		if (isPadConnected)
		{
			guideText =
				"十字ボタン：移動    X：壁モード    A：決定";
		}
		else
		{
			guideText =
				"方向キー：移動    TAB：壁モード    ENTER：決定";
		}
	}
	else
	{
		if (isPadConnected)
		{
			guideText =
				"十字ボタン：移動    Y：壁を回転    X：駒モード    A：配置";
		}
		else
		{
			guideText =
				"方向キー：移動    RSHIFT：壁を回転    TAB：駒モード    ENTER：配置";
		}
	}

	DrawCenteredText(
		screenW / 2,
		guideY + static_cast<int>(screenH * 0.024f),
		guideText.c_str(),
		colorSubText,
		fontMain_
	);

	// --------------------------------------------------
	// CPU思考中
	// --------------------------------------------------
	if (gameMode_ == GAME_MODE::CPU &&
		currentTurn_ == 1 &&
		isCpuThinking_)
	{
		const int dotCount =
			(GetNowCount() / 350) % 4;

		std::string thinkingText = "CPUが考えています";

		for (int i = 0; i < dotCount; ++i)
		{
			thinkingText += ".";
		}

		const int thinkingWidth =
			static_cast<int>(screenW * 0.25f);

		const int thinkingHeight =
			static_cast<int>(screenH * 0.06f);

		const int thinkingX =
			(screenW - thinkingWidth) / 2;

		const int thinkingY =
			turnPanelY + turnPanelHeight +
			static_cast<int>(screenH * 0.012f);

		DrawPanel(
			thinkingX,
			thinkingY,
			thinkingX + thinkingWidth,
			thinkingY + thinkingHeight,
			170
		);

		DrawCenteredText(
			screenW / 2,
			thinkingY + static_cast<int>(screenH * 0.017f),
			thinkingText.c_str(),
			colorGold,
			fontMain_
		);
	}

	// --------------------------------------------------
	// 壁不足の警告
	// --------------------------------------------------
	if (isWallWarningActive_)
	{
		const int warningWidth =
			static_cast<int>(screenW * 0.38f);

		const int warningHeight =
			static_cast<int>(screenH * 0.065f);

		const int warningX =
			(screenW - warningWidth) / 2;

		const int warningY =
			guideY -
			warningHeight -
			static_cast<int>(screenH * 0.015f);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);

		DrawBox(
			warningX,
			warningY,
			warningX + warningWidth,
			warningY + warningHeight,
			GetColor(66, 25, 22),
			TRUE
		);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		DrawBox(
			warningX,
			warningY,
			warningX + warningWidth,
			warningY + warningHeight,
			colorWarning,
			FALSE
		);

		DrawCenteredText(
			screenW / 2,
			warningY + static_cast<int>(screenH * 0.018f),
			"配置できる壁が残っていません",
			colorWarning,
			fontMain_
		);
	}

	// --------------------------------------------------
	// ポーズ画面
	// --------------------------------------------------
	if (isPause_ || pauseX_ > -320.0f)
	{
		PauseDraw();
	}
}

void Quoridor::Reset(void)
{

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
			PlaySoundMem(mCSe_, DX_PLAYTYPE_BACK);
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
				PlaySoundMem(pMSe_, DX_PLAYTYPE_BACK);
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
					PlaySoundMem(pMSe_, DX_PLAYTYPE_BACK);
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
			PlaySoundMem(wMSe_, DX_PLAYTYPE_BACK);
		}

		if (ins.IsTrgUp(KEY_INPUT_RSHIFT) || ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP))
		{
			wallVertical_ = !wallVertical_;
			PlaySoundMem(wRSe_, DX_PLAYTYPE_BACK);
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
				PlaySoundMem(wMSe_, DX_PLAYTYPE_BACK);
				mode_ = MODE::MOVE;
				isChangeTurn_ = true;
				WaitTimer(150);
			}
		}
	}
}


void Quoridor::UpdateCpu(void)
{
	constexpr int CPU_INDEX = 1;
	constexpr int SEARCH_DEPTH = 2;
	constexpr int THINKING_TIME_MS = 700;

	// CPUのターンでなければ何もしない
	if (currentTurn_ != CPU_INDEX)
	{
		isCpuThinking_ = false;
		return;
	}

	// CPU本体が生成されていない
	if (!cpu_)
	{
		return;
	}

	// 最初のフレームでは思考演出を開始する
	if (!isCpuThinking_)
	{
		isCpuThinking_ = true;
		cpuStartTime_ = GetNowCount();
		return;
	}

	// 一定時間は「CPU思考中」を表示する
	const int elapsedTime =
		GetNowCount() - cpuStartTime_;

	if (elapsedTime < THINKING_TIME_MS)
	{
		return;
	}

	QuoridorCpu::CpuAction action{};

	const bool foundAction =
		cpu_->FindBestAction(
			*board_,
			players_,
			action,
			SEARCH_DEPTH);

	if (!foundAction)
	{
#ifdef _DEBUG
		DbgLog(
			"[UpdateCpu] CPU action was not found.");
#endif // DEBUG

		isCpuThinking_ = false;
		return;
	}

	if (!ApplyCpuAction(action))
	{
#ifdef _DEBUG
		DbgLog(
			"[UpdateCpu] Failed to apply CPU action.");
#endif // DEBUG

		isCpuThinking_ = false;
		return;
	}

	isCpuThinking_ = false;

	// Update()の最後でターンを切り替える
	isChangeTurn_ = true;
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

bool Quoridor::PauseUpdate(void)
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
		}
		else
		{
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
			if (!isGameOver_)
			{
				PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP, false);
			}
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
			if (!isGameOver_)
			{
				PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP, false);
			}
			isPause_ = false;
			PlaySoundMem(cancelSe_, DX_PLAYTYPE_BACK);
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

void Quoridor::PauseDraw(void)
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

bool Quoridor::ApplyCpuAction(const QuoridorCpu::CpuAction& action)
{
	constexpr int CPU_INDEX = 1;

	Player& cpuPlayer =
		players_[CPU_INDEX];

	// ---------------------------------
	// 壁を配置する場合
	// ---------------------------------
	if (action.isWall)
	{
		if (cpuPlayer.remainingWalls_ <= 0)
		{
			DbgLog(
				"[ApplyCpuAction] CPU has no walls.");

			return false;
		}

		const bool placed =
			board_->PlaceWall(
				action.x,
				action.y,
				action.isVertical,
				players_,
				cpuPlayer.wallColor_);

		if (!placed)
		{
			DbgLog(
				"[ApplyCpuAction] Wall placement failed: (" +
				std::to_string(action.x) +
				"," +
				std::to_string(action.y) +
				")");

			return false;
		}

		cpuPlayer.remainingWalls_--;

		PlaySoundMem(
			wMSe_,
			DX_PLAYTYPE_BACK);

		mode_ = MODE::MOVE;

		DbgLog(
			"[ApplyCpuAction] WALL placed: (" +
			std::to_string(action.x) +
			"," +
			std::to_string(action.y) +
			"), vertical=" +
			std::to_string(action.isVertical));

		return true;
	}

	// ---------------------------------
	// 駒を移動する場合
	// ---------------------------------
	if (action.x < 0 ||
		action.x >= BOARD_SIZE ||
		action.y < 0 ||
		action.y >= BOARD_SIZE)
	{
		DbgLog(
			"[ApplyCpuAction] Move is outside board.");

		return false;
	}

	// 実際の盤面側でも合法手を再確認する
	const auto candidates =
		board_->GetAllMoveCandidates(
			cpuPlayer,
			players_);

	bool isLegalMove = false;

	for (const auto& candidate : candidates)
	{
		if (candidate.first == action.x &&
			candidate.second == action.y)
		{
			isLegalMove = true;
			break;
		}
	}

	if (!isLegalMove)
	{
		DbgLog(
			"[ApplyCpuAction] Illegal move rejected: (" +
			std::to_string(action.x) +
			"," +
			std::to_string(action.y) +
			")");

		return false;
	}

	cpuPlayer.x_ = action.x;
	cpuPlayer.y_ = action.y;

	PlaySoundMem(
		pMSe_,
		DX_PLAYTYPE_BACK);

	mode_ = MODE::MOVE;

	DbgLog(
		"[ApplyCpuAction] MOVE applied: (" +
		std::to_string(action.x) +
		"," +
		std::to_string(action.y) +
		")");

	return true;
}
