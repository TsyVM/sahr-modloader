#include "utils.h"
#include <ctime>
#include <iomanip>
#include <sstream>

std::ofstream g_logFile;

void InitializeLogging() {
    g_logFile.open("ModLoader.log", std::ios::out | std::ios::trunc);
    if (g_logFile.is_open()) {
        LogMessage("=== SHAR ModLoader Initialized ===");
        LogMessage("Version: 1.0.0");
        LogMessage("Build Date: " __DATE__ " " __TIME__);
    }
}

void ShutdownLogging() {
    if (g_logFile.is_open()) {
        LogMessage("=== SHAR ModLoader Shutting Down ===");
        g_logFile.close();
    }
}

void LogMessage(const std::string& message) {
    if (!g_logFile.is_open()) return;

    SYSTEMTIME st;
    GetLocalTime(&st);

    char timestamp[32];
    sprintf_s(timestamp, "[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);

    g_logFile << timestamp << message << std::endl;
    g_logFile.flush();
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();

    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);

    return result;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size <= 0) return std::wstring();

    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);

    return result;
}