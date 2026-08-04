#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct ModInfo {
    std::string name;
    std::string version;
    std::string author;
    int priority;
    fs::path rootPath;
    std::vector<std::wstring> overriddenFiles;
};

class ModManager {
public:
    ModManager();
    ~ModManager();

    void LoadMods(const fs::path& modsDirectory);
    bool HasOverride(const std::wstring& relativePath) const;
    std::wstring GetOverride(const std::wstring& relativePath) const;
    size_t GetOverrideCount() const { return m_overrides.size(); }
    void Reload();
    const std::vector<ModInfo>& GetLoadedMods() const { return m_loadedMods; }

private:
    void ScanModFolder(const fs::path& modFolder);
    void LoadManifest(const fs::path& modFolder, ModInfo& modInfo);
    void ScanModFiles(const fs::path& modFolder, ModInfo& modInfo);
    std::wstring NormalizePath(const std::wstring& path) const;

    std::unordered_map<std::wstring, std::wstring> m_overrides;
    std::vector<ModInfo> m_loadedMods;
    fs::path m_modsDirectory;
};

extern ModManager* g_pModManager;