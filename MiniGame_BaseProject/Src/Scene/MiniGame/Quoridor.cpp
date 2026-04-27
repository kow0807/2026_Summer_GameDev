#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/Setting.h"
#include "Quoridor.h"

Quoridor::Quoridor(void)
{
	players_[0] = { 4, 0, GetColor(255, 0, 0) };
	players_[1] = { 4, 8, GetColor(0, 0, 255) };

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
}

void Quoridor::Update(void)
{

	Player& player = players_[currentTurn_];	

	// ì¸óÕä÷òA
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgUp(KEY_INPUT_UP)) player.y_--;
	if (ins.IsTrgUp(KEY_INPUT_DOWN)) player.y_++;
	if (ins.IsTrgUp(KEY_INPUT_LEFT)) player.x_--;
	if (ins.IsTrgUp(KEY_INPUT_RIGHT)) player.x_++;

	// îÕàÕêßå¿
	if (player.x_ < 0) player.x_ = 0;
	if (player.x_ >= BOARD_SIZE) player.x_ = BOARD_SIZE - 1;
	if (player.y_ < 0) player.y_ = 0;
	if (player.y_ >= BOARD_SIZE) player.y_ = BOARD_SIZE - 1;

	if(ins.IsTrgUp(KEY_INPUT_RETURN))
	{
		currentTurn_ = (currentTurn_ + 1) % 2;

		// âüÇµÇ¡ÇœÇ»Çµñhé~
		WaitTimer(150);
	}
}

void Quoridor::Draw(void)
{
	DrawBoard();
	DrawPlayers();
}

void Quoridor::DrawUI(void)
{
	DrawFormatString(0, 0, GetColor(255, 255, 255),
		"Turn: Player %d", currentTurn_ + 1);
}

void Quoridor::Reset(void)
{
	// íËì_ÉJÉÅÉâ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
	SceneManager::GetInstance().GetCamera()->ChangeGameCamera(Camera::GAME_CAMERA::MOUSE);
}

VECTOR Quoridor::GetWorldPos(int x, int y)
{
	return VECTOR(x * CELL_SIZE, 0.0f, y * CELL_SIZE);
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

void Quoridor::DrawBox3D(VECTOR min, VECTOR max, unsigned int color, int fillFlag)
{

	// 8í∏ì_
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

	// ñ Ç≤Ç∆Ç…ï`âÊ
	auto drawFace = [&](int a, int b, int c, int d)
	{
			DrawTriangle3D(vertexs[a], vertexs[b], vertexs[c], color, fillFlag);
			DrawTriangle3D(vertexs[a], vertexs[c], vertexs[d], color, fillFlag);
	};

	// ëOñ 
	drawFace(0, 1, 2, 3);

	// îwñ 
	drawFace(4, 5, 6, 7);

	// ç∂
	drawFace(0, 3, 7, 4);

	// âE
	drawFace(1, 2, 6, 5);

	// è„
	drawFace(3, 2, 6, 7);

	// â∫
	drawFace(0, 1, 5, 4);
}
