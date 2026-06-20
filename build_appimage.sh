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

# Use linuxdeploy and linuxdeploy-plugin-qt instead of linuxdeployqt for Qt6
echo "Downloading linuxdeploy..."
wget -q "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" -O linuxdeploy
chmod +x linuxdeploy

echo "Downloading linuxdeploy-plugin-qt..."
wget -q "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" -O linuxdeploy-plugin-qt
chmod +x linuxdeploy-plugin-qt

echo "Bundling Qt libraries and creating AppImage..."
export QMAKE=/usr/lib/qt6/bin/qmake
if [ ! -f "$QMAKE" ]; then
    export QMAKE=$(which qmake6)
fi
export VERSION="${APP_VERSION}"
export EXTRA_QT_PLUGINS="iconengines,imageformats,tls"

./linuxdeploy --appdir "${APPDIR}" --plugin qt --output appimage

echo ""
echo "============================================"
echo "  SUCCESS! AppImage created:"
echo "  ${APP_NAME}-${APP_VERSION}-x86_64.AppImage"
echo "============================================"
