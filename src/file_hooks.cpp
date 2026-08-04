// src/file_hooks.cpp
// File redirection hook — converted from MinHook to VanHooks SDK.

#include "file_hooks.h"
#include "mod_manager.h"
#include "utils.h"
#include <vh/vh.hpp>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <optional>
#include <eh.h>
#include <exception>
#include <windows.h>

CreateFileW_t g_OriginalCreateFileW = nullptr;
thread_local bool g_inHook          = false;

static std::unordered_set<std::wstring> g_noOverrideCache;
static const size_t                     MAX_CACHE_SIZE = 1000;
static std::optional<vh::Hook>          g_createFileHook;

// ── SEH bridge ───────────────────────────────────────────────────────────────

class SEHException : public std::exception {
public:
    explicit SEHException(unsigned int code) : code_(code) {}
    unsigned int code() const { return code_; }
private:
    unsigned int code_;
};

static void SETranslator(unsigned int code, EXCEPTION_POINTERS*) {
    throw SEHException(code);
}

struct ResetInHookFlag {
    ~ResetInHookFlag() { g_inHook = false; }
};

// ── Path helpers ─────────────────────────────────────────────────────────────

static bool IsRelativePath(const std::wstring& path) {
    if (path.length() >= 2 && path[1] == L':')            return false;
    if (path.length() >= 2 && path[0] == L'\\' && path[1] == L'\\') return false;
    return true;
}

static std::wstring NormalizePath(const std::wstring& path) {
    std::wstring n = path;
    std::replace(n.begin(), n.end(), L'/', L'\\');
    if (n.length() >= 2 && n[0] == L'.' && n[1] == L'\\')
        n = n.substr(2);
    std::transform(n.begin(), n.end(), n.begin(), ::towlower);
    return n;
}

// ── Detour ───────────────────────────────────────────────────────────────────

HANDLE WINAPI HookedCreateFileW(
    LPCWSTR               lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile)
{
    // Re-entrancy guard: if we're already inside the hook, call through directly.
    if (g_inHook) {
        return g_OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }

    g_inHook = true;
    ResetInHookFlag _guard;

    _set_se_translator(SETranslator);

    try {
        if (!lpFileName || !g_pModManager) {
            return g_OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        }

        std::wstring requestedPath = lpFileName;

        // Only intercept relative paths — SHAR uses relative paths for its assets.
        if (!IsRelativePath(requestedPath)) {
            return g_OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        }

        std::wstring normalizedPath = NormalizePath(requestedPath);

        // Fast-path: already confirmed no override exists for this path.
        if (g_noOverrideCache.find(normalizedPath) != g_noOverrideCache.end()) {
            return g_OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        }

        if (g_pModManager->HasOverride(normalizedPath)) {
            std::wstring overridePath = g_pModManager->GetOverride(normalizedPath);
            LogMessage("OVERRIDE: " + WStringToString(normalizedPath) +
                       " -> " + WStringToString(overridePath));
            return g_OriginalCreateFileW(overridePath.c_str(), dwDesiredAccess, dwShareMode,
                lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        }

        // Cache the miss so we don't repeat the lookup.
        if (g_noOverrideCache.size() < MAX_CACHE_SIZE)
            g_noOverrideCache.insert(normalizedPath);

        return g_OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    catch (const SEHException& e) {
        LogMessage("EXCEPTION in CreateFileW hook! SEH code: " + std::to_string(e.code()));
    }
    catch (const std::exception& e) {
        LogMessage(std::string("STL exception in CreateFileW hook: ") + e.what());
    }
    catch (...) {
        LogMessage("UNKNOWN exception in CreateFileW hook.");
    }

    // Fallback on any exception — let the game handle the call normally.
    return g_OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// ── Init / shutdown ──────────────────────────────────────────────────────────

bool InitializeFileHooks() {
    LogMessage("Initializing file hooks (VanHooks " +
               std::string(vanhooks::VERSION_STRING) + ")...");

    auto result = vh::api_hook(
        "kernel32", "CreateFileW",
        &HookedCreateFileW,
        &g_OriginalCreateFileW,
        { .tag = "kernel32.CreateFileW" }
    );

    if (!result) {
        LogMessage("ERROR: Failed to hook CreateFileW: " +
                   std::string(vh::error_to_string(result.error())));
        return false;
    }

    g_createFileHook = std::move(*result);
    LogMessage("CreateFileW hook installed successfully.");
    return true;
}

void ShutdownFileHooks() {
    LogMessage("Shutting down file hooks...");
    if (g_createFileHook) {
        g_createFileHook->remove();
        g_createFileHook.reset();
    }
    LogMessage("File hooks removed.");
}
