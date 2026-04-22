#include <DxLib.h>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "Fader.h"

Fader::STATE Fader::GetState(void) const
{
	return state_;
}

bool Fader::IsEnd(void) const
{
	return isEnd_;
}

void Fader::SetFade(STATE state)
{
	state_ = state;
	if (state_ != STATE::NONE)
	{
		isPreEnd_ = false;
		isEnd_ = false;
	}
}

Fader::Fader(void) : resMng_(ResourceManager::GetInstance())
{
}

Fader::~Fader(void)
{
}

void Fader::Init(void)
{
	state_ = STATE::NONE;
	alpha_ = 0;
	isPreEnd_ = true;
	isEnd_ = true;
	loadingBarFrame_ = 0;
}

void Fader::Update(void)
{
	if (isEnd_)
	{
		return;
	}

	loadingBarFrame_++;

	// ローディングバーが100％を超えたか判定
	const int maxProgress = 100;
	bool isLoadingComplete = (loadingBarFrame_ >= maxProgress);



	switch (state_)
	{
	case STATE::NONE:
		return;

	case STATE::FADE_OUT:
		alpha_ += SPEED_ALPHA;
		if (alpha_ > 255)
		{
			alpha_ = 255;
			if (isPreEnd_)
			{
				// フェードアウト終了
				isEnd_ = true;
			}
			isPreEnd_ = true;
		}
		break;

	case STATE::FADE_IN:
		// ローディングバーが終わってからフェードイン開始
		if (!isLoadingComplete)
		{
			return; // バーが100％になるまではフェードインを進めない
		}

		alpha_ -= SPEED_ALPHA;
		if (alpha_ < 0)
		{
			alpha_ = 0;
			if (isPreEnd_)
			{
				isEnd_ = true;
			}
			isPreEnd_ = true;
		}
		break;

	default:
		return;
	}
}

void Fader::Draw(void)
{

	switch (state_)
	{
	case STATE::NONE:
		return;
	case STATE::FADE_OUT:
	case STATE::FADE_IN:
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)alpha_);

		// 背景の黒塗り
		DrawBox(
			0, 0,
			Application::SCREEN_SIZE_X,
			Application::SCREEN_SIZE_Y,
			0x000000, true);

		// ローディングバー描画
		if (!isEnd_)
		{
			const int barWidth = 400;
			const int barHeight = 20;
			const int cornerRadiusX = barHeight / 2;
			const int cornerRadiusY = barHeight / 2;
			const int maxProgress = 100;

			int x = (Application::SCREEN_SIZE_X - barWidth) / 2;
			int y = Application::SCREEN_SIZE_Y / 2;

			int progress = (loadingBarFrame_ < maxProgress) ? loadingBarFrame_ : maxProgress;
			float progressRatio = (float)progress / maxProgress;
			int filledWidth = static_cast<int>(barWidth * progressRatio);
			int filledRight = x + filledWidth;

			// 「Now Loading...」表示（中央上に）
			DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2 - 40, 1.0, 0.0, loadImg_, true);

			// グレーの背景バー（角丸）
			DrawRoundRect(x, y, x + barWidth, y + barHeight, cornerRadiusX, cornerRadiusY, GetColor(150, 150, 150), TRUE);


			// 赤い進行バー（角丸 or 通常矩形）
			if (filledWidth >= cornerRadiusX * 2)
			{
				// 丸みをつけた赤バー
				DrawRoundRect(x, y, filledRight, y + barHeight, cornerRadiusX, cornerRadiusY, GetColor(255, 0, 0), TRUE);

				// 角丸の影を下半分にだけ描画
				int shadowTop = y + barHeight / 2;
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100); // 半透明

				// 影の矩形（丸みは元と同じで下半分だけ塗りつぶし）
				DrawRoundRect(x, shadowTop, filledRight, y + barHeight, cornerRadiusX, cornerRadiusY, GetColor(0, 0, 0), TRUE);

				SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha_));
			}
			else if (filledWidth > 0)
			{
				// 幅が狭い → 通常矩形で描画（角丸だと崩れる）
				DrawBox(x, y, filledRight, y + barHeight, GetColor(255, 0, 0), TRUE);

				// 影（下半分に半透明黒）
				int shadowY = y + barHeight / 2;
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
				DrawBox(x, shadowY, filledRight, y + barHeight, GetColor(0, 0, 0), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha_));
			}

			// 白い枠（角丸）※塗りつぶしなし
			DrawRoundRect(x, y, x + barWidth, y + barHeight, cornerRadiusX, cornerRadiusY, GetColor(255, 255, 255), FALSE);
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;
	}

}

void Fader::SetLoadingBarFrame(int loadingBarFrame)
{
	loadingBarFrame_ = loadingBarFrame;
}
