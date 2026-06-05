cmake_minimum_required(VERSION 3.10)

if(WIN32)
    # In Qt 5.1+ we have our own main() function, don't autolink to qtmain on Windows
    cmake_policy(SET CMP0020 NEW)
endif()

project(Launcher)
enable_testing()

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BUILD_DIR}" IS_IN_SOURCE_BUILD)
if(IS_IN_SOURCE_BUILD)
    message(FATAL_ERROR "You are building the Launcher in-source. Please separate the build tree from the source tree.")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_HOST_SYSTEM_VERSION MATCHES ".*[Mm]icrosoft.*" OR
        CMAKE_HOST_SYSTEM_VERSION MATCHES ".*WSL.*"
    )
        message(FATAL_ERROR "Building the Launcher is not supported in Linux-on-Windows distributions.")
    endif()
endif()

option(Launcher_RUN_CLANG_TIDY "Run clang-tidy with the compiler." OFF)
if(Launcher_RUN_CLANG_TIDY)
  find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
  if(NOT CLANG_TIDY_COMMAND)
    message(WARNING "CMake_RUN_CLANG_TIDY is ON but clang-tidy is not found!")
    set(DO_CLANG_TIDY "" CACHE STRING "" FORCE)
  else()
    set(CLANG_TIDY_CHECKS "-modernize-use-trailing-return-type,-readability-magic-numbers,-modernize-avoid-c-arrays")
    set(DO_CLANG_TIDY "${CLANG_TIDY_COMMAND};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/src/*'")
  endif()
  # TODO: make this per-target, differentiate between libraries and main codebase
  set(CMAKE_CXX_CLANG_TIDY ${DO_CLANG_TIDY})
endif()


##################################### Set CMake options #####################################
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

# Output all executables and shared libs in the main build folder, not in subfolders.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endif()
set(CMAKE_JAVA_TARGET_OUTPUT_DIR ${PROJECT_BINARY_DIR}/jars)

######## Set compiler flags ########
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
include(GenerateExportHeader)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4251 /DQT_NO_DEPRECATED_WARNINGS=Y")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS " -Wall -pedantic -Wno-deprecated-declarations -D_GLIBCXX_USE_CXX11_ABI=0 -fstack-protector-strong --param=ssp-buffer-size=4 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE " -O3 -D_FORTIFY_SOURCE=2 ${CMAKE_CXX_FLAGS_RELEASE}")
    if(UNIX AND APPLE)
        set(CMAKE_CXX_FLAGS " -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Werror=return-type -O0")
    # Fix build with Qt 5.13
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_DEPRECATED_WARNINGS=Y")
endif()

##################################### Set Application options #####################################

######## Set URLs ########
set(Launcher_NEWS_URL "" CACHE STRING "URL to open Launcher's news.")
set(Launcher_NEWS_RSS_URL "" CACHE STRING "URL to fetch Launcher's news RSS feed from.")

######## Set version numbers ########
set(Launcher_VERSION_MAJOR    1)
set(Launcher_VERSION_MINOR    0)
set(Launcher_VERSION_HOTFIX   1)

# Build number
set(Launcher_VERSION_BUILD -1 CACHE STRING "Build number. -1 for no build number.")

# Build platform.
set(Launcher_BUILD_PLATFORM "" CACHE STRING "A short string identifying the platform that this build was built for. Only used by the notification system and to display in the about dialog.")

# Channel list URL
set(Launcher_UPDATER_BASE "" CACHE STRING "Base URL for the updater.")

# Notification URL
set(Launcher_NOTIFICATION_URL "" CACHE STRING "URL for checking for notifications.")

#### Check the current Git commit and branch
include(GetGitRevisionDescription)
get_git_head_revision(Launcher_GIT_REFSPEC Launcher_GIT_COMMIT)

message(STATUS "Git commit: ${Launcher_GIT_COMMIT}")
message(STATUS "Git refspec: ${Launcher_GIT_REFSPEC}")

set(Launcher_RELEASE_VERSION_NAME "${Launcher_VERSION_MAJOR}.${Launcher_VERSION_MINOR}.${Launcher_VERSION_HOTFIX}")

#### Custom target to just print the version.
add_custom_target(version echo "Version: ${Launcher_RELEASE_VERSION_NAME}")
add_custom_target(tcversion echo "\\#\\#teamcity[setParameter name=\\'env.LAUNCHER_VERSION\\' value=\\'${Launcher_RELEASE_VERSION_NAME}\\']")

################################ 3rd Party Libs ################################

# Find the required Qt parts
# Use QT_DIR environment variable if set, otherwise fall back to default path
if(DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
else()
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")
endif()
list(APPEND CMAKE_PREFIX_PATH "$ENV{USERPROFILE}/vcpkg/installed/x64-windows")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent Network NetworkAuth Test Xml Svg OpenGL)

if(WIN32)
    set(VCPKG_ROOT "$ENV{USERPROFILE}/vcpkg/installed/x64-windows")
    # Derive QT_BIN_DIR from Qt6 install prefix
    get_target_property(_qt6_core_dir Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(NOT _qt6_core_dir)
        get_target_property(_qt6_core_dir Qt6::Core IMPORTED_LOCATION_DEBUG)
    endif()
    if(NOT _qt6_core_dir)
        # Fallback: use Qt6_DIR which points to lib/cmake/Qt6
        get_filename_component(QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)
    else()
        get_filename_component(QT_BIN_DIR "${_qt6_core_dir}" DIRECTORY)
    endif()
    
    set(COMMON_DLLS
        "${QT_BIN_DIR}/Qt6Core.dll"
        "${QT_BIN_DIR}/Qt6Gui.dll"
        "${QT_BIN_DIR}/Qt6Network.dll"
        "${QT_BIN_DIR}/Qt6Widgets.dll"
        "${QT_BIN_DIR}/Qt6Xml.dll"
        "${QT_BIN_DIR}/Qt6Test.dll"
        "${QT_BIN_DIR}/Qt6Concurrent.dll"
        "${QT_BIN_DIR}/Qt6Svg.dll"
        "${QT_BIN_DIR}/d3dcompiler_47.dll"
        "${VCPKG_ROOT}/bin/zlib1.dll"
    )

    set(COMMON_DLLS_DEBUG
        "${QT_BIN_DIR}/Qt6Cored.dll"
        "${QT_BIN_DIR}/Qt6Guid.dll"
        "${QT_BIN_DIR}/Qt6Networkd.dll"
        "${QT_BIN_DIR}/Qt6Widgetsd.dll"
        "${QT_BIN_DIR}/Qt6Xmld.dll"
        "${QT_BIN_DIR}/Qt6Testd.dll"
        "${QT_BIN_DIR}/Qt6Concurrentd.dll"
        "${QT_BIN_DIR}/Qt6Svgd.dll"
        "${QT_BIN_DIR}/d3dcompiler_47.dll"
        "${VCPKG_ROOT}/debug/bin/zlibd1.dll"
    )

    # Choose DLLs based on build type
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DLLS_TO_DEPLOY ${COMMON_DLLS_DEBUG})
    else()
        set(DLLS_TO_DEPLOY ${COMMON_DLLS})
    endif()

    # 1. Configuration-time copy (Immediate fail-safe)
    foreach(dll ${DLLS_TO_DEPLOY})
        if(EXISTS "${dll}")
            file(COPY "${dll}" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        endif()
    endforeach()

    # 2. Build-time target (Ensures updates and dependency tracking)
    add_custom_target(DeployCommonDependencies ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLLS_TO_DEPLOY} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMENT "Deploying common Qt and vcpkg dependencies to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()

# Qt 6 provides variables for paths
set(QT_PLUGINS_DIR "${Qt6_INSTALL_PLUGINS}")
set(QT_LIBS_DIR "${Qt6_INSTALL_LIBS}")
set(QT_LIBEXECS_DIR "${Qt6_INSTALL_LIBEXECS}")

if (Qt6_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

####################################### Branding #######################################

set(Launcher_CommonName "WerkLauncher")

set(Launcher_Copyright "Vorwerkua Studio")
set(Launcher_Domain "werklauncher.com")
set(Launcher_Name "WerkLauncher")
set(Launcher_DisplayName "WerkLauncher")
set(Launcher_APP_BINARY_NAME "WerkLauncher")
set(Launcher_UserAgent "${Launcher_CommonName}/${Launcher_RELEASE_VERSION_NAME}")
set(Launcher_ConfigFile "werklauncher.cfg")
set(Launcher_Git "https://github.com/WerkLauncher/Launcher")

set(Launcher_Branding_ICNS "${PROJECT_SOURCE_DIR}/branding/${Launcher_CommonName}.icns")
set(Launcher_Branding_WindowsRC "${PROJECT_SOURCE_DIR}/branding/multimc.rc")
set(Launcher_Branding_LogoQRC "${PROJECT_SOURCE_DIR}/branding/logo.qrc")

set(Launcher_META_URL "https://meta.multimc.org/v1/") # TODO: migrate to own meta server
set(Launcher_CURSEFORGE_API_KEY "$2a$10$fDvjEOK3YoF8yMPmPnaMrewnOo3X4GYDBGoyQgYeAOM1wQQrkYsmm")
set(Launcher_PASTE_EE_API_KEY "")
set(Launcher_MSA_CLIENT_ID "c36a9fb6-4f2a-41ff-90bd-ae7cc92031eb")
set(Launcher_IMGUR_CLIENT_ID "")
set(Launcher_BUG_TRACKER_URL "https://github.com/WerkLauncher/Launcher/issues")
set(Launcher_DISCORD_URL "https://discord.gg/PmkmWHRW8G")
set(Launcher_TELEGRAM_URL "https://t.me/vorwerkua_chat")
set(Launcher_SUBREDDIT_URL "")
set(Launcher_PATREON_URL "")

####################################### Install layout #######################################

# How to install the build results
set(Launcher_LAYOUT "auto" CACHE STRING "The layout for the launcher installation (auto, win-bundle, lin-nodeps, mac-bundle)")
set_property(CACHE Launcher_LAYOUT PROPERTY STRINGS auto win-bundle lin-nodeps mac-bundle)

if(Launcher_LAYOUT STREQUAL "auto")
    if(UNIX AND APPLE)
        set(Launcher_LAYOUT_REAL "mac-bundle")
    elseif(UNIX)
        set(Launcher_LAYOUT_REAL "lin-nodeps")
    elseif(WIN32)
        set(Launcher_LAYOUT_REAL "win-bundle")
    else()
        message(FATAL_ERROR "Cannot choose a sensible install layout for your platform.")
    endif()
else()
    set(Launcher_LAYOUT_REAL ${Launcher_LAYOUT})
endif()

if(Launcher_LAYOUT_REAL STREQUAL "mac-bundle")
    set(BINARY_DEST_DIR "${Launcher_Name}.app/Contents/MacOS")
    set(LIBRARY_DEST_DIR "${Launcher_Name}.app/Contents/MacOS")
    set(PLUGIN_DEST_DIR "${Launcher_Name}.app/Contents/MacOS")
    set(RESOURCES_DEST_DIR "${Launcher_Name}.app/Contents/Resources")
    set(JARS_DEST_DIR "${Launcher_Name}.app/Contents/MacOS/jars")

    set(BUNDLE_DEST_DIR ".")

    # Apps to bundle
    set(APPS "\${CMAKE_INSTALL_PREFIX}/${Launcher_Name}.app")

    # Mac bundle settings
    set(MACOSX_BUNDLE_BUNDLE_NAME "${Launcher_Name}")
    set(MACOSX_BUNDLE_INFO_STRING "${Launcher_Name}:  Minecraft launcher and management utility.")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.multimc.${Launcher_Name}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${Launcher_VERSION_MAJOR}.${Launcher_VERSION_MINOR}.${Launcher_VERSION_HOTFIX}.${Launcher_VERSION_BUILD}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${Launcher_VERSION_MAJOR}.${Launcher_VERSION_MINOR}.${Launcher_VERSION_HOTFIX}.${Launcher_VERSION_BUILD}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${Launcher_VERSION_MAJOR}.${Launcher_VERSION_MINOR}.${Launcher_VERSION_HOTFIX}.${Launcher_VERSION_BUILD}")
    set(MACOSX_BUNDLE_ICON_FILE ${Launcher_Name}.icns)
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2015-2024 ${Launcher_Copyright}")

    # directories to look for dependencies
    set(DIRS ${QT_LIBS_DIR} ${QT_LIBEXECS_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # install as bundle
    set(INSTALL_BUNDLE "full")

    # Add the icon
    install(FILES ${Launcher_Branding_ICNS} DESTINATION ${RESOURCES_DEST_DIR} RENAME ${Launcher_Name}.icns)

elseif(Launcher_LAYOUT_REAL STREQUAL "lin-nodeps")
    set(BINARY_DEST_DIR "bin")
    set(LIBRARY_DEST_DIR "bin")
    set(PLUGIN_DEST_DIR "plugins")
    set(BUNDLE_DEST_DIR ".")
    set(RESOURCES_DEST_DIR ".")
    set(JARS_DEST_DIR "bin/jars")

    # install as bundle with no dependencies included
    set(INSTALL_BUNDLE "nodeps")

    # Set RPATH
    SET(Launcher_BINARY_RPATH "$ORIGIN/")

    # Install basic runner script
    configure_file(launcher/Launcher.in "${CMAKE_CURRENT_BINARY_DIR}/LauncherScript" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/LauncherScript" DESTINATION ${BUNDLE_DEST_DIR} RENAME ${Launcher_Name})

elseif(Launcher_LAYOUT_REAL STREQUAL "win-bundle")
    set(BINARY_DEST_DIR ".")
    set(LIBRARY_DEST_DIR ".")
    set(PLUGIN_DEST_DIR ".")
    set(BUNDLE_DEST_DIR ".")
    set(RESOURCES_DEST_DIR ".")
    set(JARS_DEST_DIR "jars")

    # Apps to bundle
    set(APPS "\${CMAKE_INSTALL_PREFIX}/${Launcher_Name}.exe")

    # directories to look for dependencies
    set(DIRS ${QT_LIBS_DIR} ${QT_LIBEXECS_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # install as bundle
    set(INSTALL_BUNDLE "full")
else()
    message(FATAL_ERROR "No sensible install layout set.")
endif()

################################ Included Libs ################################

include(ExternalProject)
set_directory_properties(PROPERTIES EP_BASE External)

option(NBT_BUILD_SHARED "Build NBT shared library" ON)
option(NBT_USE_ZLIB "Build NBT library with zlib support" OFF)
option(NBT_BUILD_TESTS "Build NBT library tests" OFF) #FIXME: fix unit tests.
set(NBT_NAME Launcher_nbt++)
set(NBT_DEST_DIR ${LIBRARY_DEST_DIR})
add_subdirectory(libraries/libnbtplusplus)

add_subdirectory(libraries/systeminfo) # system information library
add_subdirectory(libraries/hoedown) # markdown parser
add_subdirectory(libraries/launcher) # java based launcher part for Minecraft
add_subdirectory(libraries/javacheck) # java compatibility checker
add_subdirectory(libraries/xz-embedded) # xz compression
add_subdirectory(libraries/qrcode) # library for generating QR codes
add_subdirectory(libraries/quazip) # zip manipulation library
add_subdirectory(libraries/rainbow) # Qt extension for colors
add_subdirectory(libraries/iconfix) # fork of Qt's QIcon loader
add_subdirectory(libraries/LocalPeer) # fork of a library from Qt solutions
add_subdirectory(libraries/classparser) # java class file parser
add_subdirectory(libraries/optional-bare)
add_subdirectory(libraries/tomlc99) # toml parser
add_subdirectory(libraries/katabasis) # An OAuth2 library that tried to do too much

############################### Built Artifacts ###############################

add_subdirectory(buildconfig)

# NOTE: this must always be last to appease the CMake deity of quirky install command evaluation order.
add_subdirectory(launcher)

add_executable(Launcher ALIAS ${Launcher_Name})
