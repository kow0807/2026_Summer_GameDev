#pragma once
#include <windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

class PythonAI
{
public:

	PythonAI();
	~PythonAI();

	bool Start(const std::wstring& pythonExe,const std::wstring& scriptPath);
	bool IsRunning(void) const;

	// 非同期でPythonに問い合わせ
	// 結果が出たら　callback(結果のJSON文字列)が呼ばれる
	void QueryAsync(const std::string& jsonInput, std::function<void(const std::string&)> callback);

	// 結果が出ているか(ポーリング用)
	bool HasResult(void) const;

	// 結果を取り出す(HasResultがtrueのときだけ)
	std::string TakeResult(void);

	// 現在思考中か
	bool IsThinking(void) const;

	std::string SyncQuery(const std::string& jsonInput); // 内部用（ブロッキング）

	// 強制終了
	void Shutdown(void);

private:
	HANDLE hStdinWrite_ = INVALID_HANDLE_VALUE;
	HANDLE hStdoutRead_ = INVALID_HANDLE_VALUE;
	PROCESS_INFORMATION pi_ = {};
	bool running_ = false;

	std::thread workerThread_;
	std::mutex  resultMutex_;

	std::atomic<bool> isThinking_ = false;
	std::atomic<bool> hasResult_ = false;
	std::string result_;

	
};

