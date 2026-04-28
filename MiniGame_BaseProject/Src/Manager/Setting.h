#pragma once

class Setting
{
public:

	struct WindowSize
	{
		int width_;
		int height_;
	};

	// 明示的にインスタンスを生成
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static Setting& GetInstance(void);

	// 設定の初期化
	void Init(void);

	// ウィンドウサイズの取得・設定
	const WindowSize& GetWindowSize(void) const;
	void SetWindowSize(int width = 1280, int height = 720);

	// フルスクリーンの状態確認・設定
	bool IsFullScreen(void) const;
	void SetFullScreen(bool fullScreen = false);

	// モードの切り替えの初期化
	void WindowModeInit(void);

	// 入力関連
	// マウス然りスティック然り

	// BGMの音量の取得・設定
	int GetVolume(void) const;
	void SetVolume(int volume = 255);

	// SEの音量の取得・設定
	int GetSEVolume(void) const;
	void SetSEVolume(int volume =255);

	// ジョイパッドの振動の取得・設定
	bool isJoypadVibration(void) const;
	void SetJoypadVibration(bool isVibration = false);

	// リソースの削除
	void Destroy(void);

private:

	// 設定データ	
	struct SettingData
	{

	};

	// コンストラクタ
	Setting(void);

	// デストラクタ
	~Setting(void) = default;
	
	static Setting* instance_;

	// 音量
	int BgmVolume_;
	int SeVolume_;

	// ウィンドウサイズ
	WindowSize windowSize_;

	// フルスクリーンの状態
	bool isFullScreen_;

	// 解像度変更
	int fullScreenWidth_, fullScreenHeight_;

	// パッド振動
	bool isVibration_;

};
