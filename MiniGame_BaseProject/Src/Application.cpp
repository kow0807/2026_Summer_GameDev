#include <chrono>
#include <thread>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Manager/InputManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SceneManager.h"
#include "Manager/Setting.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_SHADER = "Data/Shader/";
const std::string Application::PATH_SOUND = "Data/Sound/";

unsigned int lastTime = GetNowCount(); // ミリ秒単位の現在時間

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	// アプリケーションの初期設定
	SetWindowText("ひとあそび");

	// 設定の初期化
	Setting::CreateInstance();

	// ウィンドウサイズ
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);
	ChangeWindowMode(true);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseerの初期化
	InitEffekseer();

	// キー制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();

	SetUseLighting(true);
	SetLightDirection(VGet(0.0f, -1.0f, 0.0f));
	SetLightEnable(true);

	frameCount_ = 0;
	fps_ = 0;
}

void Application::Run(void)
{

	auto& inputManager = InputManager::GetInstance();
	auto& sceneManager = SceneManager::GetInstance();

	// ゲームループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0 && !SceneManager::GetInstance().IsGameEnd())
	{
		auto frameStart = std::chrono::high_resolution_clock::now();

		// 1秒ごとにFPSを更新
		unsigned int currentTime = GetNowCount();
		frameCount_++;

		if (currentTime - lastTime >= 1000)
		{
			fps_ = frameCount_;
			frameCount_ = 0;
			lastTime = currentTime;
		}

		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

		// FPS表示（左上に白文字で）
		DrawFormatString(10, 10, GetColor(255, 255, 255), "FPS: %d", fps_);

		ScreenFlip();

		auto frameEnd = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();

		if (elapsed < FRAME_TIME)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_TIME - elapsed));
		}
	}

}

void Application::Destroy(void)
{
	Setting::GetInstance().Destroy();
	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	SceneManager::GetInstance().Destroy();

	// Effekseerを終了する。
	Effkseer_End();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	delete instance_;

}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
{
	isInitFail_ = false;
	isReleaseFail_ = false;
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}