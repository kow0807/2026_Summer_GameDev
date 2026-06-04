#include <stdexcept>
#include "../../../Manager/PythonRuntimeManager.h"
#include "PythonAI.h"

PythonAI::PythonAI() {}

PythonAI::~PythonAI()
{
    if (workerThread_.joinable())
        workerThread_.join();
    if (hStdinWrite_ != INVALID_HANDLE_VALUE) CloseHandle(hStdinWrite_);
    if (hStdoutRead_ != INVALID_HANDLE_VALUE) CloseHandle(hStdoutRead_);
    if (running_)
    {
        TerminateProcess(pi_.hProcess, 0);
        CloseHandle(pi_.hProcess);
        CloseHandle(pi_.hThread);
    }
}

bool PythonAI::Start(const std::wstring& pythonExe, const std::wstring& scriptPath)
{
    // カレントディレクトリをexeの場所に設定
    wchar_t exeDir[MAX_PATH];
    GetModuleFileNameW(nullptr, exeDir, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exeDir, L'\\');
    if (lastSlash) *lastSlash = L'\0';

    std::wstring cmd = L"\"" + pythonExe + L"\" \"" + scriptPath + L"\"";

    // 実行するコマンドとディレクトリを確認
    std::wstring dbg = L"cmd: " + cmd + L"\ndir: " + std::wstring(exeDir);

#ifdef _DEBUG
    MessageBoxW(nullptr, dbg.c_str(), L"Start確認", MB_OK);
#endif

    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };

    HANDLE hStdinRead = INVALID_HANDLE_VALUE;
    HANDLE hStdoutWrite = INVALID_HANDLE_VALUE;

    if (!CreatePipe(&hStdinRead, &hStdinWrite_, &sa, 0))
    {
        MessageBoxW(nullptr, L"CreatePipe(stdin)失敗", L"Debug", MB_OK);
        return false;
    }
    if (!CreatePipe(&hStdoutRead_, &hStdoutWrite, &sa, 0))
    {
        MessageBoxW(nullptr, L"CreatePipe(stdout)失敗", L"Debug", MB_OK);
        CloseHandle(hStdinRead);
        CloseHandle(hStdinWrite_);
        hStdinWrite_ = INVALID_HANDLE_VALUE;
        return false;
    }

    SetHandleInformation(hStdinWrite_, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hStdoutRead_, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hStdinRead;
    si.hStdOutput = hStdoutWrite;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

#ifdef _DEBUG
    DWORD flags = 0;
#else
	DWORD flags = CREATE_NO_WINDOW;
#endif

    bool ok = CreateProcessW(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        TRUE,
        flags,
        nullptr,
        exeDir,
        &si,
        &pi_
    );

    // 子プロセス側のハンドルは必ず閉じる
    CloseHandle(hStdinRead);
    CloseHandle(hStdoutWrite);

    if (!ok)
    {
        DWORD err = GetLastError();
        wchar_t msg[512];
        swprintf_s(msg, L"CreateProcessW失敗\nエラーコード: %lu\ncmd: %s\ndir: %s",
            err, cmd.c_str(), exeDir);
        MessageBoxW(nullptr, msg, L"PythonAI Error", MB_OK);

        CloseHandle(hStdinWrite_);
        CloseHandle(hStdoutRead_);
        hStdinWrite_ = INVALID_HANDLE_VALUE;
        hStdoutRead_ = INVALID_HANDLE_VALUE;
        return false;
    }

    running_ = true;
    return true;
}

bool PythonAI::IsRunning(void) const 
{ 
    return running_; 
}

std::string PythonAI::SyncQuery(const std::string& jsonInput)
{
    // Pythonプロセスが生きているか確認
    DWORD exitCode = 0;
    GetExitCodeProcess(pi_.hProcess, &exitCode);
    if (exitCode != STILL_ACTIVE)
    {
        wchar_t msg[128];
        swprintf_s(msg, L"Pythonプロセスが終了している\n終了コード: %lu", exitCode);
        MessageBoxW(nullptr, msg, L"Debug", MB_OK);
        return "";
    }

    std::string msg = jsonInput + "\n";
    DWORD written = 0;
    BOOL writeOk = WriteFile(hStdinWrite_, msg.c_str(), (DWORD)msg.size(), &written, nullptr);

    if (!writeOk || written == 0)
    {
        MessageBoxW(nullptr, L"WriteFile失敗", L"Debug", MB_OK);
        return "";
    }

    std::string result;
    char ch;
    DWORD read;
    while (ReadFile(hStdoutRead_, &ch, 1, &read, nullptr) && read > 0)
    {
        if (ch == '\n') break;
        result += ch;
    }
    return result;
}

void PythonAI::Shutdown(void)
{
	PythonRuntimeManager::GetInstance().CleanupRuntime();
}

void PythonAI::QueryAsync(const std::string& jsonInput, std::function<void(const std::string&)> callback)
{
    if (isThinking_) return;

    if (workerThread_.joinable())
        workerThread_.join();

    isThinking_ = true;
    hasResult_ = false;

    workerThread_ = std::thread([this, jsonInput, callback]()
    {
        std::string res = SyncQuery(jsonInput);
        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            result_ = res;
        }
        hasResult_ = true;
        isThinking_ = false;
        if (callback) callback(res);
    });
}

bool PythonAI::HasResult(void)  const 
{
    return hasResult_;
}

bool PythonAI::IsThinking(void) const 
{ 
    return isThinking_; 
}

std::string PythonAI::TakeResult(void)
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    hasResult_ = false;
    return result_;
}