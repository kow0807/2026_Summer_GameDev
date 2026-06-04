#include <Windows.h>
#include "PythonRuntimeManager.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <cstdlib>

#include <ShlObj.h> // Shell API
#include <ShlDisp.h>// Zipをフォルダとして扱う
#include <comdef.h>	// COM補助

PythonRuntimeManager* PythonRuntimeManager::instance_ = nullptr;

void PythonRuntimeManager::CreateInstance(void)
{
	// すでにインスタンスが存在する場合は何もしない
	if (instance_ != nullptr) return;
	instance_ = new PythonRuntimeManager();
}

PythonRuntimeManager& PythonRuntimeManager::GetInstance(void)
{
	return *instance_;
}

PythonRuntimeManager::PythonRuntimeManager(void)
{
	runTimeDir_ = L"Python\\Runtime";

	isExtracting_.store(false);
	isFinished_.store(false);
	progress_.store(0.0f);
	totalFiles_ .store(0);
	extractedFiles_.store(0);
}

bool PythonRuntimeManager::EnsureRunTime(void)
{
	// すでに展開済みなら何もしない
	if (ExistsRuntime()) return true;

	// zipが存在しない
	if(!ExistsRuntimeZip())
	{
		MessageBoxW(
			nullptr,
			L"PythonランタイムのZIPファイルが見つかりませんでした。",
			L"PythonRuntimeManager Error",
			MB_OK);

		return false;
	}

	// zip展開
	return ExtractRunTime();
}

std::wstring PythonRuntimeManager::GetPythonExePath(void) const
{
	fs::path pythonExe = 
		fs::path(runTimeDir_) / 
		L"Python-3.11.9-demo" /
		L"python.exe";

	return pythonExe.wstring();
}

bool PythonRuntimeManager::CleanupRuntime(void)
{
	try
	{
		if(fs::exists(runTimeDir_))
		{
			fs::remove_all(runTimeDir_);
		}

		return true;
	}
	catch (const fs::filesystem_error& e)
	{
		MessageBoxA(
			nullptr,
			e.what(),
			"PythonRuntimeManager Error",
			MB_OK);

		return false;
	}
}

void PythonRuntimeManager::StartExtractAsync(void)
{
	if (ExistsRuntime())
	{
		progress_.store(1.0f);
		isFinished_.store(true);
		return;
	}

	if (isExtracting_.load()) return;

	progress_.store(0.0f);
	isFinished_.store(false);
	isExtracting_.store(true);

	extractThread_ = std::thread([this]()
		{
			bool result = ExtractRunTime();

			progress_.store(result ? 1.0f : 0.0f);

			isFinished_.store(result);
			isExtracting_.store(false);
		});
}

float PythonRuntimeManager::GetProgress(void) const
{
	return progress_.load();
}

bool PythonRuntimeManager::IsExtracting(void) const
{
	return isExtracting_.load();
}

bool PythonRuntimeManager::IsFinished(void) const
{
	return isFinished_.load();
}

bool PythonRuntimeManager::ExtractRunTime(void)
{
#pragma region moto

	//fs::path zipPath =
	//	L"Python\\Python-3.11.9-demo.zip";

	//// 出力フォルダ作成
	//fs::create_directories(runTimeDir_);

	//STARTUPINFOW si{};
	//PROCESS_INFORMATION pi{};

	//si.cb = sizeof(si);
	//si.dwFlags = STARTF_USESHOWWINDOW;
	//si.wShowWindow = SW_HIDE;

	//std::wstring fullCommand =
	//	L"powershell -Command \"Expand-Archive -Path \\\""
	//	+ zipPath.wstring() +
	//	L"\\\" -DestinationPath \\\""
	//	+ runTimeDir_ +
	//	L"\\\" -Force\"";

	//BOOL result = CreateProcessW(
	//	nullptr,
	//	fullCommand.data(),
	//	nullptr,
	//	nullptr,
	//	FALSE,
	//	CREATE_NO_WINDOW,
	//	nullptr,
	//	nullptr,
	//	&si,
	//	&pi
	//);

	//if (!result)
	//{
	//	MessageBoxW(
	//		nullptr,
	//		L"Python Runtime の展開に失敗しました。",
	//		L"PythonRuntimeManager Error",
	//		MB_OK
	//	);

	//	return false;
	//}

	//// 展開プロセスの終了を待機
	//WaitForSingleObject(pi.hProcess, INFINITE);

	//CloseHandle(pi.hProcess);
	//CloseHandle(pi.hThread);

	//bool ok = ExistsRuntime();

	//return ok;

#pragma endregion

	fs::path zipPath =
		L"Python\\Python-3.11.9-demo.zip";

	// ZIP存在確認
	if (!fs::exists(zipPath))
	{
		return false;
	}

	progress_.store(0.05f);

	// 出力フォルダ作成
	fs::create_directories(runTimeDir_);

	progress_.store(0.1f);

	STARTUPINFOW si{};
	PROCESS_INFORMATION pi{};

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	std::wstring fullCommand =
		L"powershell -Command \"Expand-Archive -Path \\\""
		+ zipPath.wstring() +
		L"\\\" -DestinationPath \\\""
		+ runTimeDir_ +
		L"\\\" -Force\"";

	BOOL result = CreateProcessW(
		nullptr,
		fullCommand.data(),
		nullptr,
		nullptr,
		FALSE,
		CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&si,
		&pi
	);

	if (!result)
	{
		return false;
	}

	progress_.store(0.2f);

	float fakeProgress = 0.2f;

	// 展開完了待ち
	while (true)
	{
		DWORD wait =
			WaitForSingleObject(pi.hProcess, 100);

		// 終了
		if (wait == WAIT_OBJECT_0)
		{
			break;
		}

		// 疑似進行度
		fakeProgress += 0.005f;

		if (fakeProgress > 0.95f)
		{
			fakeProgress = 0.95f;
		}

		progress_.store(fakeProgress);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	progress_.store(0.98f);

	bool ok = ExistsRuntime();

	progress_.store(ok ? 1.0f : 0.0f);

	return ok;
}

bool PythonRuntimeManager::ExistsRuntime(void) const
{
	fs::path pythonExe=
		fs::path(runTimeDir_) /
		L"Python-3.11.9-demo" /
		L"python.exe";

	return fs::exists(pythonExe);
}

bool PythonRuntimeManager::ExistsRuntimeZip(void) const
{
	fs::path zipPath = 
		L"Python\\Python-3.11.9-demo.zip";

	return fs::exists(zipPath);
}

std::wstring PythonRuntimeManager::GetRuntimePath(void) const
{
	return runTimeDir_;
}

std::wstring PythonRuntimeManager::GetScriptPath(const std::wstring& relativePath) const
{
	fs::path path = fs::path(L"Python") / relativePath;
	return path.wstring();
}

void PythonRuntimeManager::Destroy(void)
{
	if (extractThread_.joinable())
	{
		extractThread_.join();
	}

	CleanupRuntime();

	delete instance_;
	instance_ = nullptr;
}