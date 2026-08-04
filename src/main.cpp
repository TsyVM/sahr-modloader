// src/main.cpp
#include <windows.h>
#include <dbghelp.h>
#include "mod_manager.h"
#include "file_hooks.h"
#include "utils.h"
#include <filesystem>
#include <cstdio> // for sprintf_s

#pragma comment(lib, "dbghelp.lib")

namespace fs = std::filesystem;

bool IsSafeModeEnabled() {
    wchar_t buffer[2];
    if (GetEnvironmentVariableW(L"SHAR_SAFEMODE", buffer, 2) > 0) {
        return true;
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        MessageBoxW(NULL, L"Safe mode activated. Mods disabled.",
            L"SHAR ModLoader", MB_OK | MB_ICONINFORMATION);
        return true;
    }

    return false;
}

// Renamed to avoid conflict with Windows SDK's UnhandledExceptionFilter
LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo) {
    LogMessage("CRITICAL: Unhandled exception caught!");

    char excCode[32];
    sprintf_s(excCode, "0x%08X", exceptionInfo->ExceptionRecord->ExceptionCode);
    LogMessage(std::string("Exception code: ") + excCode);

    HANDLE hFile = CreateFileW(L"ModLoaderCrash.dmp", GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = exceptionInfo;
        mdei.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
            MiniDumpNormal, &mdei, NULL, NULL);

        LogMessage("Crash dump written to ModLoaderCrash.dmp");
        CloseHandle(hFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)lpReserved; // suppress unused parameter warning

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        InitializeLogging();
        SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

        if (IsSafeModeEnabled()) {
            LogMessage("Safe mode enabled - mods disabled.");
            return TRUE;
        }

        g_pModManager = new ModManager();

        fs::path gameDir = fs::current_path();
        fs::path modsDir = gameDir / "mods";

        LogMessage("Game directory: " + gameDir.string());
        LogMessage("Mods directory: " + modsDir.string());

        g_pModManager->LoadMods(modsDir);

        if (!InitializeFileHooks()) {
            LogMessage("ERROR: Failed to initialize file hooks!");
            delete g_pModManager;
            g_pModManager = nullptr;
            return FALSE;
        }

        LogMessage("ModLoader ready. Monitoring file requests...");
        break;
    }

    case DLL_PROCESS_DETACH: {
        LogMessage("DLL_PROCESS_DETACH called.");

        ShutdownFileHooks();

        if (g_pModManager) {
            delete g_pModManager;
            g_pModManager = nullptr;
        }

        ShutdownLogging();
        break;
    }
    }

    return TRUE;
}
