#pragma once

#include <windows.h>

bool InitializeFileHooks();
void ShutdownFileHooks();

// Trampoline pointer filled by VanHooks on hook install.
typedef HANDLE(WINAPI* CreateFileW_t)(
    LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

extern CreateFileW_t g_OriginalCreateFileW;
