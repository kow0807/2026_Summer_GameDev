#pragma once
#include <string>
#include <atomic>
#include <thread>

class PythonRuntimeManager
{
public:

	// 明示的にインスタンスを生成
	static void CreateInstance(void);

	// インスタンスの取得
	static PythonRuntimeManager& GetInstance(void);

	// Pythonランタイムの準備
	bool EnsureRunTime(void);

	// Python実行ファイルのパスを取得
	std::wstring GetPythonExePath(void) const;

	// Scriptpath生成
	std::wstring GetScriptPath(const  std::wstring& relativePath) const;

	// 展開したランタイムのクリーンアップ
	bool CleanupRuntime(void);

	// Pythonランタイムの展開を非同期で開始
	void StartExtractAsync(void);

	// 展開の進行状況を0.0～1.0の範囲で取得
	float GetProgress(void) const;

	// 展開中かどうか
	bool IsExtracting(void) const;

	// 展開が完了したかどうか
	bool IsFinished(void) const;

	// リソースの削除
	void Destroy(void);

private:

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	PythonRuntimeManager(void);

	// デストラクタも同様
	~PythonRuntimeManager() = default;

	// コピーコンストラクタと代入演算子を削除して、コピーできない様にする
	PythonRuntimeManager(const PythonRuntimeManager&) = delete;

	// コピー代入演算子も削除して、コピーできない様にする
	PythonRuntimeManager& operator=(const PythonRuntimeManager&) = delete;

private:

	// 静的インスタンス
	static PythonRuntimeManager* instance_;

	// Pythonランタイムの展開
	bool ExtractRunTime(void);

	// Python実行ファイルのパス
	std::wstring runTimeDir_;

	//ランタイムの存在確認
	bool ExistsRuntime(void) const;

	// ZIPファイル存在確認
	bool ExistsRuntimeZip(void) const;

	// ランタイムパスの取得
	std::wstring GetRuntimePath(void) const;

	// 展開中かどうか
	std::atomic<bool> isExtracting_;
	
	// 展開が完了したかどうか
	std::atomic<bool> isFinished_;

	// 展開の進行状況（0.0～1.0）
	std::atomic<float> progress_; 

	// 展開処理を行うスレッド
	std::thread extractThread_;

	// 総ファイル数
	std::atomic<int> totalFiles_;

	// 完了ファイル数
	std::atomic<int> extractedFiles_;
};