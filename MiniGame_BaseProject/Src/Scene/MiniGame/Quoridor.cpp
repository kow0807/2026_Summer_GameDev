#include <queue>
#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Object/Actor/Quoridor/Desk.h"
#include "../../Object/Actor/Quoridor/Board.h"
#include "../../Object/Actor/Quoridor/Wall.h"
#include "Quoridor.h"

Quoridor::Quoridor(void)
{
	mode_ = MODE::MOVE;
	players_[0] = { 4, 0, GetColor(255, 0, 0), 0, 1 ,1,0 };
	players_[1] = { 4, 8, GetColor(0, 0, 255), 0, -1 ,-1,0 };

	currentTurn_ = 0;
	isChageTurn_ = false;

	wallCursorX_ = 0;
	wallCursorY_ = 0;
	wallVertical_ = true;
}

Quoridor::~Quoridor(void)
{
}

void Quoridor::Init(void)
{
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::MINI_GAME);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::NONE);
	SceneManager::GetInstance().GetCamera()->ChangeGameTypeCamera(Camera::GAME_TYPE::QUORIDOR);

	desk_ = std::make_unique<Desk>();
	desk_->Init();
	desk_->SetPosition({50.0f, -50.0f, 230.0f});

	board_ = std::make_unique<Board>();
	board_->Init();

	previewWall_ = std::make_unique<Wall>();
	previewWall_->Init();
	previewWall_->SetType(Wall::TYPE::VERTICAL);
}

void Quoridor::Update(void)
{

	desk_->Update();

	//-----------------------------------------------------------
	Player& player = players_[currentTurn_];

	// 入力関連
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgUp(KEY_INPUT_TAB))
	{
		if (mode_ == MODE::MOVE)mode_ = MODE::WALL;
		else if (mode_ == MODE::WALL) mode_ = MODE::MOVE;

		WaitTimer(150);
	}

	if (mode_ == MODE::MOVE)
	{

		int DirX = 0;
		int DirY = 0;

		// --------------------------------
		// 入力方向

		if (ins.IsTrgUp(KEY_INPUT_UP)) DirY += player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_DOWN)) DirY -= player.forwardDirY_;
		if (ins.IsTrgUp(KEY_INPUT_LEFT)) DirX -= player.rightDirX_;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT)) DirX += player.rightDirX_;

		// 相手プレイヤー取得
		Player& enemy = players_[(currentTurn_ + 1) % 2];

		if (DirX != 0 ||
			DirY != 0)
		{
			// ジャンプ判定
			int nextX = player.x_ + DirX;
			int nextY = player.y_ + DirY;

			// 壁確認
			if (!board_->CanMove(player.x_, player.y_, DirX, DirY))
			{
				return;
			}

			// 隣に敵がいるか
			if (nextX == enemy.x_ &&
				nextY == enemy.y_)
			{
				if (board_->CanMove(nextX, nextY, DirX, DirY))
				{
					// 敵のさらに先の位置
					player.x_ = player.x_ + DirX * 2;
					player.y_ = player.y_ + DirY * 2;
				}
			}
			else
			{
				// 通常移動
				player.x_ = nextX;
				player.y_ = nextY;
			}

			isChageTurn_ = true;

		}

	}
	else if (mode_ == MODE::WALL)
	{

		if (ins.IsTrgUp(KEY_INPUT_UP))    wallCursorY_--;
		if (ins.IsTrgUp(KEY_INPUT_DOWN))  wallCursorY_++;
		if (ins.IsTrgUp(KEY_INPUT_LEFT))  wallCursorX_--;
		if (ins.IsTrgUp(KEY_INPUT_RIGHT)) wallCursorX_++;

		// 向き切替
		if (ins.IsTrgUp(KEY_INPUT_RSHIFT))
		{
			wallVertical_ = !wallVertical_;
		}

		// 範囲制限
		if (wallVertical_)
		{
			if (wallCursorX_ < 0) wallCursorX_ = 0;
			if (wallCursorX_ >= BOARD_SIZE - 1)
				wallCursorX_ = BOARD_SIZE - 2;

			if (wallCursorY_ < 0) wallCursorY_ = 0;
			if (wallCursorY_ >= BOARD_SIZE)
				wallCursorY_ = BOARD_SIZE - 1;
		}
		else
		{
			if (wallCursorX_ < 0) wallCursorX_ = 0;
			if (wallCursorX_ >= BOARD_SIZE)
				wallCursorX_ = BOARD_SIZE - 1;

			if (wallCursorY_ < 0) wallCursorY_ = 0;
			if (wallCursorY_ >= BOARD_SIZE - 1)
				wallCursorY_ = BOARD_SIZE - 2;

		}

		// 向き更新
		previewWall_->SetType(
			wallVertical_
			?
			Wall::TYPE::VERTICAL
			:
			Wall::TYPE::HORIZONTAL
		);

		// 座標更新
		previewWall_->SetBoardPosition(
			wallCursorX_,
			wallCursorY_
		);

		previewWall_->RefreshTransform();

		// 壁設置
		if (ins.IsTrgUp(KEY_INPUT_RETURN))
		{
			board_->PlaceWall(
				wallCursorX_,
				wallCursorY_,
				wallVertical_,
				players_
			);

			mode_ = MODE::MOVE;

			isChageTurn_ = true;

			WaitTimer(150);
		}
	}

	// 範囲制限
	if (player.x_ < 0) player.x_ = 0;
	if (player.x_ >= BOARD_SIZE) player.x_ = BOARD_SIZE - 1;
	if (player.y_ < 0) player.y_ = 0;
	if (player.y_ >= BOARD_SIZE) player.y_ = BOARD_SIZE - 1;

	if(isChageTurn_)
	{
		currentTurn_ = (currentTurn_ + 1) % 2;
		isChageTurn_ = false;
	}

}

void Quoridor::Draw(void)
{
	desk_->Draw();
	DrawBoard();
	DrawPlayers();
	DrawWall();
	
	//if(mode_==MODE::WALL)DrawWallCursor();
	
	if (mode_ == MODE::WALL)
	{
		bool canPlace = board_->CanPlaceWall(
			wallCursorX_,
			wallCursorY_,
			wallVertical_
		);

		previewWall_->DrawPreview(canPlace);

	}
}

void Quoridor::DrawUI(void)
{
	DrawFormatString(0, 0, GetColor(255, 255, 255),
		"Turn: Player %d", currentTurn_ + 1);

	// モード表示
	const char* modeText =
		(mode_ == MODE::MOVE) ? "MOVE" : "WALL";

	DrawFormatString(0, 32, GetColor(0, 0, 0),
		"MODE : %s", modeText);
}

void Quoridor::Reset(void)
{
	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);
}

VECTOR Quoridor::GetWorldPos(int x, int y)
{
	return VECTOR(x * CELL_SIZE, 0.0f, y * CELL_SIZE);
}

VECTOR Quoridor::GetCellCenter(int x, int y)
{
	return VGet(x * CELL_SIZE, 0.0f, y * CELL_SIZE);
}

void Quoridor::DrawBoard(void)
{
	for (int y = 0; y < BOARD_SIZE; y++)
	{
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			VECTOR pos = GetWorldPos(x, y);
			DrawBox3D(
				VAdd(pos, VGet(-4, 0, -4)),
				VAdd(pos, VGet(4, 1, 4)),
				GetColor(200, 200, 200),
				TRUE
			);

			DrawBox3D(
				VAdd(pos, VGet(-4, 0, -4)),
				VAdd(pos, VGet(4, 1, 4)),
				GetColor(255, 255, 255),
				FALSE
			);
		}
	}
}

void Quoridor::DrawPlayers(void)
{
	for (int i = 0; i < 2; i++)
	{
		VECTOR pos = GetWorldPos(players_[i].x_, players_[i].y_);
		DrawSphere3D(
			VAdd(pos, VGet(0, 2, 0)),
			5.0f,
			16,
			players_[i].color_,
			players_[i].color_,
			true
		);
	}
}

void Quoridor::DrawWall(void)
{
	board_->DrawWalls();
}

void Quoridor::DrawWallCursor(void)
{
	constexpr float WALL_THICKNESS = 10.0f;
	constexpr float WALL_HEIGHT = 40.0f;
	constexpr float WALL_Y = 2.0f;
	constexpr float WALL_MARGIN = 15.0f;


	bool canPlace =
		board_->CanPlaceWall(
			wallCursorX_,
			wallCursorY_,
			wallVertical_
		);

	unsigned int color =
		canPlace ?
		GetColor(0, 255, 0) :
		GetColor(255, 0, 0);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

	//----------------------------------------
	// 縦壁

	if (wallVertical_)
	{
		VECTOR pos = VAdd(
			GetWorldPos(
				wallCursorX_,
				wallCursorY_
			),
			VGet(
				CELL_SIZE / 2,
				WALL_Y,
				CELL_SIZE / 2
			)
		);

		VECTOR min = VAdd(
			pos,
			VGet(
				-WALL_THICKNESS,
				WALL_Y,
				-CELL_SIZE + WALL_MARGIN
			)
		);

		VECTOR max = VAdd(
			pos,
			VGet(
				WALL_THICKNESS,
				WALL_HEIGHT,
				CELL_SIZE - WALL_MARGIN
			)
		);

		DrawCube3D(
			min,
			max,
			color,
			TRUE
		);
	}
	//----------------------------------------
	// 横壁
	else
	{
		VECTOR pos = VAdd(
			GetWorldPos(
				wallCursorX_,
				wallCursorY_
			),
			VGet(
				CELL_SIZE / 2,
				WALL_Y,
				CELL_SIZE / 2
			)
		);

		VECTOR min = VAdd(
			pos,
			VGet(
				-CELL_SIZE + WALL_MARGIN,
				WALL_Y,
				-WALL_THICKNESS
			)
		);

		VECTOR max = VAdd(
			pos,
			VGet(
				CELL_SIZE - WALL_MARGIN,
				WALL_HEIGHT,
				WALL_THICKNESS
			)
		);

		DrawCube3D(
			min,
			max,
			color,
			TRUE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

VECTOR Quoridor::MakeMin(VECTOR a, VECTOR b)
{
	return VGet(
		min(a.x, b.x),
		min(a.y, b.y),
		min(a.z, b.z)
	);
}

VECTOR Quoridor::MakeMax(VECTOR a, VECTOR b)
{
	return VGet(
		max(a.x, b.x),
		max(a.y, b.y),
		max(a.z, b.z)
	);
}

void Quoridor::DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{

	// 8頂点
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

	// 面ごとに描画
	auto drawFace = [&](int a, int b, int c, int d)
	{
			DrawTriangle3D(vertexs[a], vertexs[b], vertexs[c], color, fillFlag);
			DrawTriangle3D(vertexs[a], vertexs[c], vertexs[d], color, fillFlag);
	};

	// 前面
	drawFace(0, 1, 2, 3);

	// 背面
	drawFace(4, 5, 6, 7);

	// 左
	drawFace(0, 3, 7, 4);

	// 右
	drawFace(1, 2, 6, 5);

	// 上
	drawFace(3, 2, 6, 7);

	// 下
	drawFace(0, 1, 5, 4);
}

void Quoridor::DrawCube3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{
	//----------------------------------------
	// 頂点

	VECTOR v[8] =
	{
		// 前面
		VGet(min.x, min.y, min.z), // 0
		VGet(max.x, min.y, min.z), // 1
		VGet(max.x, max.y, min.z), // 2
		VGet(min.x, max.y, min.z), // 3

		// 背面
		VGet(min.x, min.y, max.z), // 4
		VGet(max.x, min.y, max.z), // 5
		VGet(max.x, max.y, max.z), // 6
		VGet(min.x, max.y, max.z), // 7
	};

	//----------------------------------------
	// 四角形描画関数

	auto DrawQuad =
	[&](
		int a,
		int b,
		int c,
		int d
	)
	{
		DrawTriangle3D(
			v[a],
			v[b],
			v[c],
			color,
			fillFlag
		);

		DrawTriangle3D(
			v[a],
			v[c],
			v[d],
			color,
			fillFlag
		);
	};

	//----------------------------------------
	// 前面

	DrawQuad(0, 1, 2, 3);

	//----------------------------------------
	// 背面
	// 順番逆転

	DrawQuad(5, 4, 7, 6);

	//----------------------------------------
	// 左面

	DrawQuad(4, 0, 3, 7);

	//----------------------------------------
	// 右面

	DrawQuad(1, 5, 6, 2);

	//----------------------------------------
	// 上面

	DrawQuad(3, 2, 6, 7);

	//----------------------------------------
	// 底面

	DrawQuad(4, 5, 1, 0);
}
