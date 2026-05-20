#include <queue>
#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Object/Actor/Quoridor/Desk.h"
#include "../../Object/Actor/Quoridor/Board.h"
#include "../../Object/Actor/Quoridor/Wall.h"
#include "../../Object/Actor/Quoridor/PlayerPiece.h"
#include "Quoridor.h"

Quoridor::Quoridor(void)
{
	gameMode_ = GAME_MODE::NONE;
	mode_ = MODE::MOVE;

	// プレイヤー0: 上側(y=0)スタート、赤、目標 y=8
	players_[0] = { 4, 0, 1.0f, 0.31f, 0.31f, 0, 1, 1, 0, MAX_WALLS };

	// プレイヤー1: 下側(y=8)スタート、青、目標 y=0
	players_[1] = { 4, 8, 0.31f, 0.31f, 1.0f, 0, -1, -1, 0, MAX_WALLS };

	currentTurn_ = 0;
	isChangeTurn_ = false;
	wallCursorX_ = 0;
	wallCursorY_ = 0;
	wallVertical_ = true;
	isGameOver_ = false;
	winner_ = -1;
}

Quoridor::~Quoridor(void)
{
}

void Quoridor::Init(void)
{
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FREE);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::QUORIDOR);

	gameMode_ = GAME_MODE::CPU;

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
}

void Quoridor::Update(void)
{
	if (isGameOver_) return;

	desk_->Update();

	Player& player = players_[currentTurn_];
	auto& ins = InputManager::GetInstance();

	// TAB でモード切替
	if (ins.IsTrgUp(KEY_INPUT_TAB))
	{
		if (mode_ == MODE::MOVE) mode_ = MODE::WALL;
		else                     mode_ = MODE::MOVE;
		WaitTimer(150);
		RefreshMoveCandidates();
	}

	// 壁モードで壁がなくなったら自動で移動モードに切り替える
	if(player.remainingWalls_ <= 0 && mode_ == MODE::WALL)
	{
		mode_ = MODE::MOVE;
		WaitTimer(150);
		RefreshMoveCandidates();
	}

	// -----------------------------------------------------------
	// 移動モード
	if (mode_ == MODE::MOVE)
	{
		int DirX = 0;
		int DirY = 0;

		if (ins.IsTrgUp(KEY_INPUT_UP))    DirY += player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_DOWN))  DirY -= player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_LEFT))  DirX -= player.rightDirX_;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT)) DirX += player.rightDirX_;

		if (DirX != 0 || DirY != 0)
		{
			// 候補リストから有効な移動先を取得
			auto cands = board_->GetMoveCandidates(
				player.x_, player.y_,
				DirX, DirY,
				players_
			);

			if (!cands.empty())
			{
				// 候補が複数ある（斜めジャンプ2方向）場合は最初の1つを選ぶ
				player.x_ = cands[0].first;
				player.y_ = cands[0].second;
				isChangeTurn_ = true;
			}
		}
	}
	// -----------------------------------------------------------
	// 壁モード
	else if (mode_ == MODE::WALL)
	{
		if (ins.IsTrgUp(KEY_INPUT_UP))    wallCursorY_ += player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_DOWN))  wallCursorY_ -= player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_LEFT))  wallCursorX_ -= player.rightDirX_;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT)) wallCursorX_ += player.rightDirX_;

		// 向き切替
		if (ins.IsTrgUp(KEY_INPUT_RSHIFT))
		{
			wallVertical_ = !wallVertical_;
		}

		// 範囲制限
		if (wallCursorX_ < 0)                 wallCursorX_ = 0;
		if (wallCursorX_ >= BOARD_SIZE - 1)   wallCursorX_ = BOARD_SIZE - 2;
		if (wallCursorY_ < 0)                 wallCursorY_ = 0;
		if (wallCursorY_ >= BOARD_SIZE - 1)   wallCursorY_ = BOARD_SIZE - 2;

		// 向き・座標更新
		previewWall_->SetType(
			wallVertical_ ? Wall::TYPE::VERTICAL : Wall::TYPE::HORIZONTAL
		);
		previewWall_->SetBoardPosition(wallCursorX_, wallCursorY_);
		previewWall_->RefreshTransform();

		// 壁設置
		if (ins.IsTrgUp(KEY_INPUT_RETURN))
		{
			// 残数チェック
			if (player.remainingWalls_ <= 0) return;

			bool placed = board_->PlaceWall(
				wallCursorX_,
				wallCursorY_,
				wallVertical_,
				players_
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

	for(int i = 0; i < 2; ++i)
	{
		playerPieces_[i]->SetBoardPosition(players_[i].x_, players_[i].y_);
	}

	// -----------------------------------------------------------
	// 範囲制限（念のため）
	if (player.x_ < 0)             player.x_ = 0;
	if (player.x_ >= BOARD_SIZE)   player.x_ = BOARD_SIZE - 1;
	if (player.y_ < 0)             player.y_ = 0;
	if (player.y_ >= BOARD_SIZE)   player.y_ = BOARD_SIZE - 1;

	// -----------------------------------------------------------
	// 勝利判定
	winner_ = board_->CheckWinner(players_);
	if (winner_ >= 0)
	{
		isGameOver_ = true;
		return;
	}

	// -----------------------------------------------------------
	// ターン交代
	if (isChangeTurn_)
	{
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

		// 点滅
		if (IsBlink())
		{
			previewWall_->DrawPreview(canPlace);
		}
	}
}

void Quoridor::DrawUI(void)
{
	// ゲームオーバー表示
	if (isGameOver_)
	{
		DrawGameOver();
		return;
	}

	// ターン表示
	DrawFormatString(0, 0, GetColor(255, 255, 255),
		"Turn: Player %d", currentTurn_ + 1);

	// モード表示
	const char* modeText =
		(mode_ == MODE::MOVE) ? "MOVE" : "WALL";
	DrawFormatString(0, 20, GetColor(200, 200, 200),
		"MODE : %s  [TAB]", modeText);

	// 壁残数表示
	DrawFormatString(0, 44, GetColor(255, 100, 100),
		"P1 Walls: %d / %d", players_[0].remainingWalls_, MAX_WALLS);
	DrawFormatString(0, 64, GetColor(100, 100, 255),
		"P2 Walls: %d / %d", players_[1].remainingWalls_, MAX_WALLS);

	// 壁モード時の補助テキスト
	if (mode_ == MODE::WALL)
	{
		DrawFormatString(0, 88, GetColor(220, 220, 0),
			"RSHIFT: rotate  ENTER: place wall");
	}
}

void Quoridor::Reset(void)
{
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);

	mode_ = MODE::MOVE;

	// プレイヤー0: 上側(y=0)スタート、赤、目標 y=8
	players_[0] = { 4, 0, 255.0f, 80.0f, 80.0f, 0, 1, 1, 0, MAX_WALLS };
	// プレイヤー1: 下側(y=8)スタート、青、目標 y=0
	players_[1] = { 4, 8, 80.0f, 80.0f, 255.0f, 0, -1, -1, 0, MAX_WALLS };

	currentTurn_ = 0;
	isChangeTurn_ = false;
	wallCursorX_ = 0;
	wallCursorY_ = 0;
	wallVertical_ = true;
	isGameOver_ = false;
	winner_ = -1;
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

			// ゴールラインを色分け
			unsigned int cellColor = GetColor(200, 200, 200);
			if (y == BOARD_SIZE - 1) cellColor = GetColor(255, 160, 160); // P1ゴール
			if (y == 0)              cellColor = GetColor(160, 160, 255); // P2ゴール

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
		//　自分のターン中の駒を点滅
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
