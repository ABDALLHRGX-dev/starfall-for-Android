# Starfall — Android (ARM64)

**Target artifact:** `Starfall.so` (Android ARM64 / arm64-v8a)

> This repository contains a restructured and rewritten version of the original Starfall project, targeted specifically at Android (arm64). You can run the build on **Windows**, **Linux**, or **macOS**, but the produced binary is an **Android ARM64 shared object**.

---

## Table of contents

* [Overview](#overview)
* [Compatibility (UE ↔ Fortnite)](#compatibility-ue--fortnite)
* [Repository layout](#repository-layout)
* [Engine selection (](#engine-selection-engineassociation)[`EngineAssociation`](#engine-selection-engineassociation)[)](#engine-selection-engineassociation)
* [Backend configuration (request.cpp)](#backend-configuration-requestcpp)
* [Prerequisites](#prerequisites)
* [Build instructions](#build-instructions)

  * [Windows (Build.bat)](#windows-buildbat)
  * [Linux / macOS (RunUAT)](#linux--macos-runuat)
  * [Editor-based build (optional)](#editor-based-build-optional)
* [Expected outputs (paths)](#expected-outputs-paths)
* [Troubleshooting](#troubleshooting)
* [Contributing & Attribution](#contributing--attribution)
* [License & Policy](#license--policy)
* [Contact](#contact)

---

## Overview

Starfall (this fork: **Starfall for Android**) is a C++ Unreal Engine module reorganized under `Source/Starfall`. The module was rebuilt and adapted to produce an Android ARM64 shared library suitable for Fortnite-targeted workflows. The codebase requires compilation with a matching Unreal Engine version to avoid ABI/API mismatches.

**Important:** the final binary is **Android arm64-v8a** only. Do not expect Windows/Linux/macOS binaries from this repo — they are not supported targets.

---

## Compatibility (UE ↔ Fortnite)

Use this mapping (as available in the repo) to pick the correct Unreal Engine version for your target Fortnite build:

| Unreal Engine Version | Fortnite Build Range                        |
| --------------------- | ------------------------------------------- |
| UE4.21                | 5.2.0 - 6.31                                |
| UE4.22                | 7.00 - 8.00                                 |
| UE4.23                | 8.01 - 10.40                                |
| UE4.24                | 11.00 - 12.20                               |
| UE4.25                | 12.31 - 12.61                               |
| UE4.26                | 13.00 - 16.30, 17.00 - 17.30, 18.00 - 18.21 |
| UE4.26.1              | 16.40, 16.50, 17.40, 17.50, 18.30, 18.40    |
| UE5.0                 | 19.00 - 23.00                               |
| UE5.1                 | 23.10 - N/A                                 |
| UE5.2 — UE5.6         | N/A                                         |

> Always build with the exact engine version required by the target Fortnite build. Small differences (patches/commits) can break ABI compatibility.

---

## Repository layout (required)

```
Starfall/
├─ Starfall.uproject
└─ Source/
   └─ Starfall/
      ├─ Starfall.Build.cs
      ├─ Public/
      │  ├─ base.h
      │  ├─ exit.h
      │  ├─ hooking.h
      │  ├─ opts.h
      │  ├─ pch.h
      │  ├─ redirection.h
      │  ├─ request.h
      │  ├─ ue.h
      │  ├─ UELoader.h
      │  ├─ UnrealForward.h
      │  └─ url.h
      └─ Private/
         ├─ base.cpp
         ├─ exit.cpp
         ├─ hooking.cpp
         ├─ main.cpp
         ├─ pch.cpp
         ├─ pf_patch.cpp
         ├─ redirection.cpp
         ├─ request.cpp
         ├─ ue.cpp
         ├─ UELoader.cpp
         ├─ UnrealForward.cpp
         └─ url.cpp
```

The project expects to be built as a module under `Source/Starfall`. Keep headers in `Public/` and implementation in `Private/` as shown.

---

## Engine selection (`EngineAssociation`)

Open `Starfall.uproject` and ensure the `EngineAssociation` entry specifies the engine that will be used to compile the module. This value selects the engine when opening the project with the Launcher or Editor.

Example (line \~3 in `Starfall.uproject`):

```json
{
  "FileVersion": 3,
  "EngineAssociation": "5.0",
  "Modules": [ { "Name": "Starfall", "Type": "Runtime" } ]
}
```

Set `EngineAssociation` to the exact engine version you will use (e.g., `"4.26.1"`, `"5.0"`). Incorrect engine association is the most common cause of compile/link errors.

---

## Backend configuration (request.cpp)

In `Source/Starfall/Private/request.cpp` you will find two lines (approximately lines **406** and **408**) that set the backend address:

```cpp
Backend = FString(TEXT("127.0.0.1:3551"));
Starfall::backend = FString(TEXT("127.0.0.1:3551"));
```

This default points to a local lawinserver instance. You can change the backend address using any of the options below.

### Option A — Hardcode (quick)

Replace the literal with the desired host\:port and rebuild:

```cpp
Backend = FString(TEXT("mybackend.example.com:9000"));
Starfall::backend = FString(TEXT("mybackend.example.com:9000"));
```

### Option B — Environment variable (recommended)

Let the runtime override the default without recompilation (works on editor or packaged builds where the environment is controllable):

```cpp
FString EnvBackend = FPlatformMisc::GetEnvironmentVariable(TEXT("STARFALL_BACKEND"));
if (!EnvBackend.IsEmpty()) {
    Backend = EnvBackend;
    Starfall::backend = EnvBackend;
} else {
    Backend = FString(TEXT("127.0.0.1:3551"));
    Starfall::backend = FString(TEXT("127.0.0.1:3551"));
}
```

### Option C — Config file (recommended for packaged builds)

Read from an INI file shipped with the package:

**Config/DefaultStarfall.ini**

```ini
[Starfall]
Backend=10.0.0.5:9000
```

C++ to load:

```cpp
FString IniBackend;
if (GConfig->GetString(TEXT("Starfall"), TEXT("Backend"), IniBackend, GGameIni) && !IniBackend.IsEmpty()) {
    Backend = IniBackend;
    Starfall::backend = IniBackend;
}
```

### Option D — Build-time define (least flexible)

Add a definition in `Starfall.Build.cs`:

```csharp
PublicDefinitions.Add("STARFALL_BACKEND=\"127.0.0.1:3551\"");
```

Use in C++:

```cpp
#ifdef STARFALL_BACKEND
Backend = FString(UTF8_TO_TCHAR(STARFALL_BACKEND));
Starfall::backend = FString(UTF8_TO_TCHAR(STARFALL_BACKEND));
#endif
```

---

## Prerequisites

* **Unreal Engine**: install the exact engine version you will build against (see the compatibility table).
* **Android toolchain**: Android SDK, Android NDK and Java JDK compatible with the chosen UE version. Configure `ANDROID_HOME` and `ANDROID_NDK_HOME` environment variables.
* **Host C++ toolchain**:

  * **Windows**: Visual Studio 2019/2022 (C++ workload)
  * **Linux**: Clang/LLVM toolchain supported by your UE build
  * **macOS**: Xcode command-line tools
* (Optional) `git` to clone and manage the repository

> Even if you build on Windows, Linux, or macOS, always target **Android/arm64** in the build commands.

---

## Build instructions

Open a terminal and `cd` to the repository root (where `Starfall.uproject` is located):

```bash
cd /path/to/Starfall
```

### Windows (Build.bat example)

```powershell
"C:\Path\To\UnrealEngine\Engine\Build\BatchFiles\Build.bat" Starfall Android Development -project="C:\Path\To\Starfall\Starfall.uproject" -target=Starfall -waitmutex
```

Notes:

* Replace `C:\Path\To\UnrealEngine` with your local engine path.
* Use `Android` platform and `Development` or `Shipping` configuration as appropriate.

### Linux / macOS (RunUAT example)

```bash
/Path/To/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildTarget \
  -project="/Path/To/Starfall/Starfall.uproject" \
  -platform=Android \
  -configuration=Development
```

Notes:

* Ensure `RunUAT.sh` and your engine install support building Android targets on your host OS.

### Editor-based build (optional)

1. Open `Starfall.uproject` with the correct engine editor.
2. Let the Editor compile the module automatically when prompted.
3. Use **File → Package Project → Android (ASTC/ETC2)** if you want the engine to package a full APK (not usually required for producing the `.so`).

---

## Expected outputs (paths)

After a successful build you should find the Android ARM64 shared library:

```
Starfall/Binaries/Android/Starfall.so
```

Or grouped under ABI-specific foldering:

```
Starfall/Binaries/Android/arm64-v8a/Starfall.so
```

---

## Troubleshooting

* **Undefined symbols / link errors**: usually caused by an engine version mismatch. Confirm `EngineAssociation` and the engine you used to build.
* **NDK or SDK errors**: verify `ANDROID_HOME` and `ANDROID_NDK_HOME` and the NDK version expected by your UE version.
* **API differences UE4 ↔ UE5**: add conditional compilation guards (e.g., `#if ENGINE_MAJOR_VERSION >= 5`).
* **Run-time load failure**: ensure the `.so` is compiled for `arm64-v8a` and packaged under the correct path used by your app.

---

## Contributing & Attribution

This project is a restructured Android-focused fork of the original Starfall repository. If you publish or distribute builds, you **must** follow the repository policy `LICENSE` file.

Minimum attribution (example):

> Based on Starfall (original by plooshi) and restructured/rewritten for Android by ABDALLHRGX-dev.

---

## License & Policy

This repo includes a `LICENSE` file (plain text) containing redistribution and attribution rules. Read them before using, redistributing, or publishing binaries.

---

## Contact

* **Author / maintainer**: ABDALLHRGX-dev — [https://github.com/ABDALLHRGX-dev](https://github.com/ABDALLHRGX-dev)
* Discord: `abdallhgxr`
* X / Twitter: [https://x.com/THEFOUNDATASAS](https://x.com/THEFOUNDATASAS)

---
