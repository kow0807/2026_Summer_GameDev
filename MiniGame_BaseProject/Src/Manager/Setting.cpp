#include "../Manager/Setting.h"

Setting* Setting::instance_ = nullptr;

Setting::Setting(void)
	:
	isFullScreen_(false),
	windowSize_{ 1280, 720 },
	BgmVolume_(255),
	SeVolume_(255),
	fullScreenWidth_(1920),
	fullScreenHeight_(1080)
{
}

void Setting::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Setting();
	}
	instance_->Init();
}

Setting& Setting::GetInstance(void)
{
	return *instance_;
}

void Setting::Init(void)
{
}

const Setting::WindowSize& Setting::GetWindowSize(void) const
{
	return windowSize_;
}

void Setting::SetWindowSize(int width, int height)
{
}

bool Setting::IsFullScreen(void) const
{
	return false;
}

void Setting::SetFullScreen(bool fullScreen)
{
}

void Setting::WindowModeInit(void)
{
}

int Setting::GetVolume(void) const
{
	return 0;
}

void Setting::SetVolume(int volume)
{
}

int Setting::GetSEVolume(void) const
{
	return 0;
}

void Setting::SetSEVolume(int volume)
{
}

bool Setting::isJoypadVibration(void) const
{
	return false;
}

void Setting::SetJoypadVibration(bool isVibration)
{
}

void Setting::Destroy(void)
{
	delete instance_;
}