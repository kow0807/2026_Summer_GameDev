#include <Windows.h>
#include <DxLib.h>
#include "../Manager/Setting.h"

Setting* Setting::instance_ = nullptr;

Setting::Setting(void)
	:
	isFullScreen_(false),
	windowSize_{ 1024, 640 },
	BgmVolume_(255),
	SeVolume_(255),
	fullScreenWidth_(1920),
	fullScreenHeight_(1080),
	isVibration_(false)
{
}

void Setting::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Setting();
		instance_->Init();
	}
}

Setting& Setting::GetInstance(void)
{
	return *instance_;
}

void Setting::Init(void)
{
	if (!isFullScreen_)
	{
		windowSize_.width_ = 1024;
		windowSize_.height_ = 640;
	}
	SetGraphMode(windowSize_.width_, windowSize_.height_, 32);
	ChangeWindowMode(true);
}

const Setting::WindowSize& Setting::GetWindowSize(void) const
{
	return windowSize_;
}

void Setting::SetWindowSize(int width, int height)
{
	windowSize_.width_ = width;
	windowSize_.height_ = height;
	WindowModeInit();
}

bool Setting::IsFullScreen(void) const
{
	return isFullScreen_;
}

void Setting::SetFullScreen(bool fullScreen)
{
	isFullScreen_ = fullScreen;
	WindowModeInit();
}

void Setting::WindowModeInit(void)
{
	SetDrawScreen(DX_SCREEN_BACK);

	if (!isFullScreen_) 
	{
		SetGraphMode(windowSize_.width_, windowSize_.height_, 32);
		ChangeWindowMode(true);
	}
	else
	{
		fullScreenWidth_ = GetSystemMetrics(SM_CXSCREEN);
		fullScreenHeight_ = GetSystemMetrics(SM_CYSCREEN);
		ChangeWindowMode(false);

		SetFullScreenResolutionMode(DX_FSRESOLUTIONMODE_DESKTOP);	
	}
}

int Setting::GetBGMVolume(void) const
{
	return BgmVolume_;
}

void Setting::SetBGMVolume(int volume)
{
	BgmVolume_ = volume;
}

int Setting::GetSEVolume(void) const
{
	return SeVolume_;
}

void Setting::SetSEVolume(int volume)
{
	SeVolume_ = volume;
}

bool Setting::isJoypadVibration(void) const
{
	return isVibration_;
}

void Setting::SetJoypadVibration(bool isVibration)
{
	isVibration_ = isVibration;
	SetUseJoypadVibrationFlag(isVibration_);
}

void Setting::Destroy(void)
{
	delete instance_;
}