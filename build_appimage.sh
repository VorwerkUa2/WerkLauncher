#!/bin/bash
# ============================================
#   WerkLauncher AppImage Builder
#   For Linux Mint / Ubuntu / Debian
# ============================================

set -e

APP_NAME="WerkLauncher"
APP_VERSION="1.0.1"
BUILD_DIR="${BUILD_DIR:-out/build/x64-Release}"
APPDIR="AppDir"

echo "============================================"
echo "  ${APP_NAME} AppImage Builder"
echo "============================================"
echo ""

# Check dependencies
if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Install with: sudo apt install cmake"
    exit 1
fi

# Build Release if not already built
if [ ! -f "${BUILD_DIR}/${APP_NAME}" ]; then
    echo "Building Release..."
    cmake -S . -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build "${BUILD_DIR}"
    echo ""
fi

# Clean previous AppDir
rm -rf "${APPDIR}"

# Create AppDir structure
echo "Creating AppDir structure..."
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/lib"
mkdir -p "${APPDIR}/usr/share/applications"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/256x256/apps"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/scalable/apps"

# Copy executable
cp "${BUILD_DIR}/${APP_NAME}" "${APPDIR}/usr/bin/"

# Copy JARs
mkdir -p "${APPDIR}/usr/bin/jars"
cp "${BUILD_DIR}"/jars/*.jar "${APPDIR}/usr/bin/jars/"

# Copy shared libraries built by the project
cp "${BUILD_DIR}"/*.so* "${APPDIR}/usr/lib/" 2>/dev/null || true

# Copy desktop file and icon
cp WerkLauncher.desktop "${APPDIR}/"
cp WerkLauncher.desktop "${APPDIR}/usr/share/applications/"

# Use SVG icon if available, fallback to converting ICO
if [ -f "branding/logo.svg" ]; then
    cp "branding/logo.svg" "${APPDIR}/usr/share/icons/hicolor/scalable/apps/werklauncher.svg"
    cp "branding/logo.svg" "${APPDIR}/werklauncher.svg"
fi

# Ensure qmake is found by linuxdeployqt
export PATH="/usr/lib/qt6/bin:$PATH"
if command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    mkdir -p /tmp/qmake-bin
    ln -s $(which qmake6) /tmp/qmake-bin/qmake
    export PATH="/tmp/qmake-bin:$PATH"
fi

# Download linuxdeployqt if not available
if ! command -v linuxdeployqt &> /dev/null; then
    if [ ! -f "linuxdeployqt" ]; then
        echo "Downloading linuxdeployqt..."
        wget -q "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
        chmod +x linuxdeployqt
    fi
    LINUXDEPLOYQT="./linuxdeployqt"
else
    LINUXDEPLOYQT="linuxdeployqt"
fi

# Bundle Qt libraries and create AppImage
echo "Bundling Qt libraries and creating AppImage..."
export VERSION="${APP_VERSION}"
${LINUXDEPLOYQT} "${APPDIR}/usr/share/applications/WerkLauncher.desktop" \
    -unsupported-allow-new-glibc \
    -appimage \
    -verbose=1

echo ""
echo "============================================"
echo "  SUCCESS! AppImage created:"
echo "  ${APP_NAME}-${APP_VERSION}-x86_64.AppImage"
echo "============================================"
