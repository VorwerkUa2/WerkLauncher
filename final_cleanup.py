
import re

cpp_path = r'c:\Users\knyaz\Desktop\MCL\launcher\ui\MainWindow.cpp'
h_path = r'c:\Users\knyaz\Desktop\MCL\launcher\ui\MainWindow.h'

# 1. Update Header: Rename trayIconActivated and ensure slots are there
with open(h_path, 'r', encoding='utf-8') as f:
    h_content = f.read()

h_content = h_content.replace('trayIconActivated(QSystemTrayIcon::ActivationReason', 'on_trayIconActivated(QSystemTrayIcon::ActivationReason')

# Add on_actionREDDIT_triggered if missing
if 'on_actionREDDIT_triggered()' not in h_content:
    h_content = h_content.replace('void on_actionAbout_triggered();', 'void on_actionAbout_triggered();\n  void on_actionREDDIT_triggered();')

with open(h_path, 'w', encoding='utf-8') as f:
    f.write(h_content)

# 2. Update CPP: deduplicate, rename, fix syntax
with open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_lines = f.readlines()

# Rename in connect calls and definitions
new_cpp_lines = []
for line in cpp_lines:
    new_line = line.replace('&MainWindow::trayIconActivated', '&MainWindow::on_trayIconActivated')
    new_line = new_line.replace('void MainWindow::trayIconActivated', 'void MainWindow::on_trayIconActivated')
    new_cpp_lines.append(new_line)

content = ''.join(new_cpp_lines)

# Remove duplicate function bodies
def remove_duplicates(text, func_name):
    pattern = re.compile(rf'void\s+MainWindow::{func_name}\s*\(\)\s*\{{.*?\}}', re.DOTALL)
    matches = list(pattern.finditer(text))
    if len(matches) > 1:
        print(f"Removing {len(matches)-1} duplicates of {func_name}")
        parts = []
        last_end = 0
        for i, match in enumerate(matches):
            parts.append(text[last_end:match.start()])
            if i == 0:
                parts.append(match.group(0))
            last_end = match.end()
        parts.append(text[last_end:])
        return ''.join(parts)
    return text

content = remove_duplicates(content, 'on_actionTELEGRAM_triggered')
content = remove_duplicates(content, 'on_actionDISCORD_triggered')

# Ensure bodies exist for required actions
def ensure_body(text, func_name, body):
    if f'void MainWindow::{func_name}()' not in text:
        return text + f"\n\nvoid MainWindow::{func_name}() {{\n{body}\n}}\n"
    return text

content = ensure_body(content, 'on_actionTELEGRAM_triggered', '    DesktopServices::openUrl(QUrl(BuildConfig.TELEGRAM_URL));')
content = ensure_body(content, 'on_actionDISCORD_triggered', '    DesktopServices::openUrl(QUrl(BuildConfig.DISCORD_URL));')
content = ensure_body(content, 'on_actionREDDIT_triggered', '    DesktopServices::openUrl(QUrl(BuildConfig.REDDIT_URL));')

# Fix the mess at on_trayIconActivated
# We'll just replace the whole body of on_trayIconActivated to be sure
new_tray_body = """void MainWindow::on_trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (this->isVisible()) {
            this->hide();
        } else {
            this->showNormal();
            this->activateWindow();
        }
    }
}"""

content = re.sub(r'void\s+MainWindow::on_trayIconActivated\s*\(\s*QSystemTrayIcon::ActivationReason\s+reason\s*\)\s*\{.*?\}', new_tray_body, content, flags=re.DOTALL)

with open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(content)

print("Final cleanup done.")
