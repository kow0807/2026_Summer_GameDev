#include <queue>
#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "../../Object/Actor/Quoridor/Desk.h"
#include "../../Object/Actor/Quoridor/Board.h"
#include "Quoridor.h"

Quoridor::Quoridor(void)
{
	players_[0] = { 4, 0, GetColor(255, 0, 0), 0, 1 ,1,0 };
	players_[1] = { 4, 8, GetColor(0, 0, 255), 0, -1 ,-1,0 };

	currentTurn_ = 0;
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
}

void Quoridor::Update(void)
{

	desk_->Update();

	//-----------------------------------------------------------
	Player& player = players_[currentTurn_];

	// 入力関連
	auto& ins = InputManager::GetInstance();

	// 移動方向
	int DirX = 0;
	int DirY = 0;

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

	}

	// 設置確認
	if (ins.IsTrgUp(KEY_INPUT_1))
	{
		board_->PlaceWall(player.x_, player.y_, true, players_);
	}
	if(ins.IsTrgUp(KEY_INPUT_2))
	{
		board_->PlaceWall(player.x_, player.y_, false, players_);
	}

	// 範囲制限
	if (player.x_ < 0) player.x_ = 0;
	if (player.x_ >= BOARD_SIZE) player.x_ = BOARD_SIZE - 1;
	if (player.y_ < 0) player.y_ = 0;
	if (player.y_ >= BOARD_SIZE) player.y_ = BOARD_SIZE - 1;


	if(ins.IsTrgUp(KEY_INPUT_RETURN))
	{
		currentTurn_ = (currentTurn_ + 1) % 2;

		// 押しっぱなし防止
		WaitTimer(150);
	}
}

void Quoridor::Draw(void)
{

	desk_->Draw();
	DrawBoard();
	DrawPlayers();
	DrawWall();
}

void Quoridor::DrawUI(void)
{
	DrawFormatString(0, 0, GetColor(255, 255, 255),
		"Turn: Player %d", currentTurn_ + 1);
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
			2.0f,
			16,
			players_[i].color_,
			players_[i].color_,
			true
		);
	}
}

void Quoridor::DrawWall(void)
{
	int x, y;
	x = 0;
	y = 0;

	// 縦壁
	VECTOR vPos = VAdd(
		GetCellCenter(x, y), 
		VGet(CELL_SIZE / 2.0f, 0, 0));

	// 横壁
	VECTOR hPos=VAdd(GetCellCenter(x,y), VGet(0, 0, CELL_SIZE / 2.0f));
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
