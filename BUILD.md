# Building WerkLauncher

WerkLauncher is a C++17/Qt 6 application built with CMake. The project includes
its supporting libraries in `libraries/`; clone the repository with its
submodules so all dependencies are available.

## Requirements

- CMake 3.10 or newer
- A C++17 compiler (MSVC, Clang, or GCC)
- Qt 6 development packages for Core, Widgets, Concurrent, Network,
  NetworkAuth, Test, Xml, Svg, OpenGL, Quick, Qml, QuickControls2, and
  QuickWidgets
- zlib and a Java development kit for the launcher support tools
- Ninja is recommended for command-line builds

Windows builds also require a matching Qt 6 kit and vcpkg with the project's
zlib dependency. Set `QT_DIR` to the Qt installation prefix when Qt is not
installed in the default location. On Windows, CMake finds vcpkg through
`VCPKG_INSTALLATION_ROOT` or the default `%USERPROFILE%\\vcpkg` location.

## Configure and build

Run these commands from the repository root, using an out-of-source build
directory:

```sh
cmake -S . -B out/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/build
ctest --test-dir out/build --output-on-failure
```

For Windows, provide the vcpkg toolchain file when configuring:

```powershell
cmake -S . -B out/build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build out/build
ctest --test-dir out/build -C Release --output-on-failure
```

If needed, pass `-DLauncher_CURSEFORGE_API_KEY=<key>` during configuration to
enable CurseForge API access. The key is optional and is not embedded in source
control by default.

## Packaging

- Windows: build the release target and compile `installer.iss` with Inno Setup
  6. The `build_release.bat` script is available for the configured Visual
  Studio/vcpkg environment.
- Linux: run `build_appimage.sh` after installing the Qt deployment tools used
  by the script.
- macOS: build the app bundle, then use Qt's `macdeployqt` to create the DMG.

For the CI dependency lists and platform packaging steps, see
`.github/workflows/build.yml`.
