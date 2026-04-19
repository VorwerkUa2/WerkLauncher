; WerkLauncher Inno Setup Script
; ==============================
; Встановіть Inno Setup 6 перед компіляцією цього скрипта

#define MyAppName "WerkLauncher"
#define MyAppVersion "1.0.1"
#define MyAppPublisher "Vorwerkua Studio"
#define MyAppURL "https://werklauncher.com"
#define MyAppExeName "WerkLauncher.exe"

; Шлях до скомпільованого Release білда
#define BuildDir "out\build\x64-Release"

[Setup]
; Унікальний ID додатку (не змінювати для майбутніх оновлень!)
AppId={{E6F7C8A9-2B3D-4A5B-9C8D-7E6F5A4B3C2D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; Шлях за замовчуванням
DefaultDirName={autopf}\{#MyAppName}
; Назва теки в меню Пуск
DefaultGroupName={#MyAppName}

; Де зберігати готовий інсталятор
OutputDir=out\installer
OutputBaseFilename=WerkLauncher_Setup_{#MyAppVersion}

; Іконка інсталятора (той самий що і в програми)
SetupIconFile=branding\MultiMC.ico
UninstallDisplayIcon={app}\{#MyAppExeName}

; -------- КАСТОМІЗАЦІЯ ДИЗАЙНУ --------
; Розміри для зображень:
; 1. WizardImageFile: 164x314 пікселів (велика картинка зліва на першому екрані)
; 2. WizardSmallImageFile: 55x55 пікселів (маленьке лого справа зверху на інших екранах)
;
; Формат МАЄ БУТИ .bmp (Bitmap)!
; Окремо розкоментуйте ці рядки, коли створите зображення:
;
WizardImageFile=branding\installer_sidebar.bmp
WizardSmallImageFile=branding\installer_header.bmp
; --------------------------------------

; Ліцензія
LicenseFile=COPYING.md

; Сучасний стиль сторінок
WizardStyle=modern
Compression=lzma2/ultra64
SolidCompression=yes

[Languages]
; Вбудований український та англійський переклад інсталятора
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone

[Files]
; ---- Головний файл та іконка ----
Source: "{#BuildDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "branding\MultiMC.ico"; DestDir: "{app}\branding"; Flags: ignoreversion createallsubdirs

; ---- Qt6 DLLs (Release) ----
Source: "{#BuildDir}\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Core5Compat.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Xml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Concurrent.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion

; ---- Інші DLLs ----
Source: "{#BuildDir}\Launcher_iconfix.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion

; ---- Опціональні DLLs для графіки/локалізації (ігнорувати, якщо немає) ----
Source: "{#BuildDir}\dxcompiler.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist
Source: "{#BuildDir}\dxil.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist
Source: "{#BuildDir}\icuuc.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist

; ---- Qt Плагіни ----
Source: "{#BuildDir}\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#BuildDir}\imageformats\*.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "{#BuildDir}\iconengines\*.dll"; DestDir: "{app}\iconengines"; Flags: ignoreversion
Source: "{#BuildDir}\styles\*.dll"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "{#BuildDir}\tls\*.dll"; DestDir: "{app}\tls"; Flags: ignoreversion
Source: "{#BuildDir}\networkinformation\*.dll"; DestDir: "{app}\networkinformation"; Flags: ignoreversion
Source: "{#BuildDir}\generic\*.dll"; DestDir: "{app}\generic"; Flags: ignoreversion

; ---- JAR файли ----
Source: "{#BuildDir}\jars\*.jar"; DestDir: "{app}\jars"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\branding\MultiMC.ico"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\branding\MultiMC.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
