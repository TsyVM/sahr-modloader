#include "mod_manager.h"
#include "utils.h"
#include <algorithm>
#include <fstream>

ModManager* g_pModManager = nullptr;

ModManager::ModManager() {
}

ModManager::~ModManager() {
}

void ModManager::LoadMods(const fs::path& modsDirectory) {
    m_modsDirectory = modsDirectory;
    
    if (!fs::exists(modsDirectory)) {
        LogMessage("Mods directory does not exist: " + modsDirectory.string());
        try {
            fs::create_directory(modsDirectory);
            LogMessage("Created mods directory.");
        } catch (const std::exception& e) {
            LogMessage("Failed to create mods directory: " + std::string(e.what()));
        }
        return;
    }

    LogMessage("Loading mods from: " + modsDirectory.string());

    std::vector<fs::path> modFolders;
    try {
        for (const auto& entry : fs::directory_iterator(modsDirectory)) {
            if (entry.is_directory()) {
                modFolders.push_back(entry.path());
            }
        }
    } catch (const std::exception& e) {
        LogMessage("Error scanning mods directory: " + std::string(e.what()));
        return;
    }

    std::sort(modFolders.begin(), modFolders.end(),
        [](const fs::path& a, const fs::path& b) {
            return a.filename().string() < b.filename().string();
        });

    for (const auto& modFolder : modFolders) {
        try {
            ScanModFolder(modFolder);
        } catch (const std::exception& e) {
            LogMessage("Error loading mod " + modFolder.filename().string() + ": " + e.what());
        }
    }

    LogMessage("Total overrides loaded: " + std::to_string(m_overrides.size()));
}

void ModManager::ScanModFolder(const fs::path& modFolder) {
    LogMessage("Scanning mod folder: " + modFolder.filename().string());

    ModInfo modInfo;
    modInfo.rootPath = modFolder;
    modInfo.name = modFolder.filename().string();
    modInfo.priority = 0;
    modInfo.version = "unknown";
    modInfo.author = "unknown";

    fs::path manifestPath = modFolder / "mod.json";
    if (fs::exists(manifestPath)) {
        LoadManifest(manifestPath, modInfo);
    }

    ScanModFiles(modFolder, modInfo);

    m_loadedMods.push_back(modInfo);
}

void ModManager::LoadManifest(const fs::path& manifestPath, ModInfo& modInfo) {
    try {
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            LogMessage("WARNING: Could not open manifest: " + manifestPath.string());
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        size_t namePos = content.find("\"name\"");
        if (namePos != std::string::npos) {
            size_t startQuote = content.find("\"", namePos + 7);
            size_t endQuote = content.find("\"", startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                modInfo.name = content.substr(startQuote + 1, endQuote - startQuote - 1);
            }
        }

        size_t versionPos = content.find("\"version\"");
        if (versionPos != std::string::npos) {
            size_t startQuote = content.find("\"", versionPos + 10);
            size_t endQuote = content.find("\"", startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                modInfo.version = content.substr(startQuote + 1, endQuote - startQuote - 1);
            }
        }

        size_t authorPos = content.find("\"author\"");
        if (authorPos != std::string::npos) {
            size_t startQuote = content.find("\"", authorPos + 9);
            size_t endQuote = content.find("\"", startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                modInfo.author = content.substr(startQuote + 1, endQuote - startQuote - 1);
            }
        }

        LogMessage("  Loaded manifest: " + modInfo.name + " v" + modInfo.version + " by " + modInfo.author);

    } catch (const std::exception& e) {
        LogMessage("ERROR parsing manifest: " + std::string(e.what()));
    }
}

void ModManager::ScanModFiles(const fs::path& modFolder, ModInfo& modInfo) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(modFolder)) {
            if (!entry.is_regular_file()) continue;

            if (entry.path().filename() == "mod.json" || 
                entry.path().filename() == "preview.png") {
                continue;
            }

            fs::path relPath = fs::relative(entry.path(), modFolder);
            std::wstring normalizedPath = NormalizePath(relPath.wstring());

            m_overrides[normalizedPath] = entry.path().wstring();
            modInfo.overriddenFiles.push_back(normalizedPath);

            LogMessage("  Override: " + WStringToString(normalizedPath));
        }
    } catch (const std::exception& e) {
        LogMessage("Error scanning mod files: " + std::string(e.what()));
    }
}

std::wstring ModManager::NormalizePath(const std::wstring& path) const {
    std::wstring normalized = path;
    
    std::replace(normalized.begin(), normalized.end(), L'/', L'\\');
    
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::towlower);
    
    return normalized;
}

bool ModManager::HasOverride(const std::wstring& relativePath) const {
    std::wstring normalized = NormalizePath(relativePath);
    return m_overrides.find(normalized) != m_overrides.end();
}

std::wstring ModManager::GetOverride(const std::wstring& relativePath) const {
    std::wstring normalized = NormalizePath(relativePath);
    auto it = m_overrides.find(normalized);
    if (it != m_overrides.end()) {
        return it->second;
    }
    return L"";
}

void ModManager::Reload() {
    m_overrides.clear();
    m_loadedMods.clear();
    LoadMods(m_modsDirectory);
}