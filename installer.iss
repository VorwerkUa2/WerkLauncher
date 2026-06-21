; WerkLauncher Inno Setup Script
; ==============================
; Встановіть Inno Setup 6 перед компіляцією цього скрипта

#define MyAppName "WerkLauncher"
#define MyAppVersion "1.1.5"
#define MyAppPublisher "Vorwerkua Studio"
#define MyAppURL "https://werklauncher.com"
#define MyAppExeName "WerkLauncher.exe"

; Шлях до скомпільованого Release білда
#ifndef BuildDir
#define BuildDir "out\build\x64-Release"
#endif

[Setup]
; Унікальний ID додатку (не змінювати для майбутніх оновлень!)
AppId={{E6F7C8A9-2B3D-4A5B-9C8D-7E6F5A4B3C2D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; Осучаснення вигляду
WizardStyle=modern
WizardSizePercent=110
DisableWelcomePage=no

; Шлях за замовчуванням
DefaultDirName={autopf}\{#MyAppName}
; Назва теки в меню Пуск
DefaultGroupName={#MyAppName}

; Де зберігати готовий інсталятор
OutputDir=out\installer
OutputBaseFilename=WerkLauncher_Setup_{#MyAppVersion}

; Іконка інсталятора (той самий що і в програми)
SetupIconFile=branding\WerkLauncher.ico
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
Source: "branding\WerkLauncher.ico"; DestDir: "{app}\branding"; Flags: ignoreversion

; ---- Qt6 DLLs (Release) ----
Source: "{#BuildDir}\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Core5Compat.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6NetworkAuth.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Xml.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Concurrent.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; ---- Інші DLLs ----
Source: "{#BuildDir}\Launcher_iconfix.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\z.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; ---- Опціональні DLLs для графіки/локалізації (ігнорувати, якщо немає) ----
Source: "{#BuildDir}\dxcompiler.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist
Source: "{#BuildDir}\dxil.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist
Source: "{#BuildDir}\icuuc.dll"; DestDir: "{app}"; Flags: skipifsourcedoesntexist

; ---- Qt Плагіни ----
Source: "{#BuildDir}\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\imageformats\*.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\iconengines\*.dll"; DestDir: "{app}\iconengines"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\styles\*.dll"; DestDir: "{app}\styles"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\tls\*.dll"; DestDir: "{app}\tls"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\networkinformation\*.dll"; DestDir: "{app}\networkinformation"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\generic\*.dll"; DestDir: "{app}\generic"; Flags: ignoreversion skipifsourcedoesntexist

; ---- JAR файли ----
Source: "{#BuildDir}\jars\*.jar"; DestDir: "{app}\jars"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\branding\WerkLauncher.ico"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\branding\WerkLauncher.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
procedure InitializeWizard;
begin
  // Встановлюємо темний колір фону та білий текст
  WizardForm.Color := $202020;
  WizardForm.Font.Color := clWhite;
  
  WizardForm.WelcomePage.Color := $202020;
  WizardForm.InnerPage.Color := $202020;
  WizardForm.FinishedPage.Color := $202020;
  WizardForm.LicensePage.Color := $202020;
  WizardForm.SelectDirPage.Color := $202020;
  WizardForm.SelectProgramGroupPage.Color := $202020;
  WizardForm.SelectTasksPage.Color := $202020;
  WizardForm.ReadyPage.Color := $202020;
  WizardForm.PreparingPage.Color := $202020;
  WizardForm.InstallingPage.Color := $202020;
  
  WizardForm.MainPanel.Color := $252525;
  WizardForm.PageNameLabel.Font.Color := clWhite;
  WizardForm.PageDescriptionLabel.Font.Color := $DDDDDD;
  
  // Виправлення тексту на головних сторінках
  WizardForm.WelcomeLabel1.Font.Color := clWhite;
  WizardForm.WelcomeLabel2.Font.Color := clWhite;
  WizardForm.FinishedHeadingLabel.Font.Color := clWhite;
  WizardForm.FinishedLabel.Font.Color := clWhite;
  
  // Виправлення текстових полів (Ліцензія та Підсумок)
  WizardForm.LicenseMemo.Color := $252525;
  WizardForm.LicenseMemo.Font.Color := clWhite;
  WizardForm.ReadyMemo.Color := $252525;
  WizardForm.ReadyMemo.Font.Color := clWhite;
  
  // Виправлення списку програм, що працюють
  WizardForm.PreparingMemo.Color := $252525;
  WizardForm.PreparingMemo.Font.Color := clWhite;
  
  // Виправлення перемикачів ліцензії
  WizardForm.LicenseAcceptedRadio.Font.Color := clWhite;
  WizardForm.LicenseNotAcceptedRadio.Font.Color := clWhite;
  
  // Виправлення перемикачів "Закрити програми"
  WizardForm.PreparingYesRadio.Font.Color := clWhite;
  WizardForm.PreparingNoRadio.Font.Color := clWhite;
  
  // Виправлення списку завдань (іконок)
  WizardForm.TasksList.Color := $252525;
  WizardForm.TasksList.Font.Color := clWhite;
end;
