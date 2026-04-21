<div align="center">
  <br>
  <img src="branding/logo.svg" alt="WerkLauncher Logo" width="128">
  <h1>WerkLauncher</h1>
  <p><i>A Modern, High-Performance, and Heavily Customized Minecraft Launcher</i></p>

  [![Build](https://github.com/WerkLauncher/Launcher/actions/workflows/build.yml/badge.svg)](https://github.com/WerkLauncher/Launcher/actions)
  [![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
  [![Discord](https://img.shields.io/discord/123456789?label=Discord&logo=discord&logoColor=white)](https://discord.gg/PmkmWHRW8G)
</div>

<hr>

## 🌐 Language / Мова
[English](#-english-description) | [Українська](#-український-опис)

<hr>

## 🇺🇸 English Description

**WerkLauncher** is a sophisticated, performance-oriented fork of the [MultiMC](https://multimc.org/) project. Designed for power users who demand a sleeker interface, native OS integration, and seamless support for custom authentication systems.

### ✨ Key Features
- **Premium UI Experience**:
  - **Native Custom Title Bar**: Supports Windows **Aero Snap**, high-DPI scaling, and native dragging on Linux/macOS.
  - **Dynamic Theming**: Interface automatically adapts to Dark, Fusion, and Bright modes with consistent branding.
  - **Optimized Layout**: Replaced the bulky SidePanel with a smart `instanceToolBar` that only appears when needed.
- **Advanced Authentication**:
  - **Modern MSA Flow**: Fully utilizes the Browser-based Authorization Code flow for Microsoft Accounts (migrated from outdated Device Flow).
  - **Custom Ecosystem**: Built-in support for **Minecraft Skin System Server (MSSS)** with Yggdrasil API and RSA key handling.
- **Technical Excellence**:
  - Accelerated instance grid rendering.
  - Native installer support via Inno Setup.
  - Fully automated CI/CD for Windows, Linux, and macOS.

### 🛠️ Building from Source
#### Windows (Recommended)
Uses `Ninja` and `Vcpkg` for ultra-fast builds.
```bash
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg.cmake]
cmake --build build
```

<hr>

## 🇺🇦 Український опис

**WerkLauncher** — це сучасний, оптимізований та кастомний форк лаунчера [MultiMC](https://multimc.org/). Він поєднує в собі потужність оригіналу з преміальним дизайном та глибокою інтеграцією в екосистему кастомних серверів.

### ✨ Ключові зміни
- **Преміальний інтерфейс**:
  - **Кастомний заголовок вікна**: Повна підтримка **Windows Aero Snap**, нативного перетягування та коректного масштабування на 4K моніторах.
  - **Розумна панель інструментів**: Прибрано застарілу бічну панель; натомість додано `instanceToolBar`, який автоматично ховається, коли не вибрано жодної збірки.
- **Безпечна та сучасна авторизація**:
  - **Microsoft Accounts**: Перехід на сучасний браузерний метод авторизації (Authorization Code Flow), що забезпечує максимальну безпеку.
  - **MSSS Інтеграція**: Повна підтримка власної системи скінів через Yggdrasil API.
- **Стабільність та швидкість**:
  - Оптимізовано рендеринг списку ігор.
  - Автоматизована збірка (CI/CD) для всіх популярних ОС.

### 🛠️ Як зібрати проєкт
#### Windows
```bash
# Debug версія:
build_multimc.bat

# Релізна версія:
build_release.bat
```

<hr>

<div align="center">
  <p>Made with ❤️ by the WerkLauncher Team</p>
  <p><i>Licensed under Apache 2.0. See COPYING.md for more details.</i></p>
</div>
