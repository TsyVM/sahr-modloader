#pragma once

#include <string>
#include <fstream>
#include <windows.h>

void InitializeLogging();
void ShutdownLogging();
void LogMessage(const std::string& message);

std::string WStringToString(const std::wstring& wstr);
std::wstring StringToWString(const std::string& str);

extern std::ofstream g_logFile;