# SHAR ModLoader

> ⚠️ **Work in Progress** — this is a rebuilt version of the old SHAR ModLoader. It may not be fully stable. The legacy loader is still available if you need something confirmed working.

A mod loader for *The Simpsons: Hit & Run*, built on [VanHooks](https://github.com/tsyvm/vanhooks). It hooks the game's file system calls and transparently redirects them to files from your `mods/` folder.

---

## Requirements

- **[ASI Ultimate Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)** — drops the mod loader into the game process automatically. No injector needed.
- A copy of *The Simpsons: Hit & Run* (PC, 32-bit)

---

## Installation

1. Install ASI Ultimate Loader into your SHAR game folder if you haven't already (follow its readme).
2. Download `SHARModLoader.asi` from the [Releases](../../releases) page.
3. Copy `SHARModLoader.asi` into your SHAR game folder.
4. Create a `mods` folder in the game folder.
5. Run the game — the mod loader is active automatically.

Hold **Shift** while the game loads to enable Safe Mode (mods disabled for that session).

Check `ModLoader.log` in the game folder if something isn't working.

---

## Adding Mods

Place each mod in its own numbered subfolder inside `mods/`. Mods are loaded in folder name order, so use a number prefix to control priority:

```
Simpsons.exe
SHARModLoader.asi
mods/
  01_my_skin/
    art/
      character/
        homer.rcs
  02_custom_music/
    sound/
      ...
```

A mod folder can contain any files that mirror the game's directory structure. When the game tries to open a file, the mod loader checks for an override first and serves the mod file if one exists.

---

## Building from Source

**Requirements:** CMake 3.25+, Visual Studio 2022 (17.6+) with the C++ Desktop workload.

Run `build.bat`. Output: `build\bin\SHARModLoader.asi`.

The VanHooks SDK is bundled in `vanhooks/` — no internet connection needed to build.

---

## Project Structure

```
src/                 Mod loader source
  main.cpp           DllMain — init, safe mode, crash dump
  mod_manager.cpp/h  Scans mods/ and builds the override table
  file_hooks.cpp/h   CreateFileW hook via VanHooks
  utils.cpp/h        Logging
CMakeLists.txt
build.bat
```
