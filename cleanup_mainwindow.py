
import re

file_path = r'c:\Users\knyaz\Desktop\MCL\launcher\ui\MainWindow.cpp'

with open(file_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Clean up empty lines and whitespace
cleaned_lines = []
for line in lines:
    stripped = line.rstrip()
    if not stripped and cleaned_lines and not cleaned_lines[-1]:
        continue
    cleaned_lines.append(stripped)

content = '\n'.join(cleaned_lines)

# Deduplicate functions
# We look for MainWindow::FunctionName(...) {
# This is a bit tricky with re, but let's try to find common ones.
functions_to_dedupe = [
    'on_actionTELEGRAM_triggered',
    'on_actionDISCORD_triggered',
    'on_actionREDDIT_triggered'
]

for func in functions_to_dedupe:
    pattern = re.compile(rf'void\s+MainWindow::{func}\s*\(\)\s*\{{.*?\}}', re.DOTALL)
    matches = list(pattern.finditer(content))
    if len(matches) > 1:
        # Keep only the first one, replace others with empty string or comment
        # We'll just replace the whole file content by removing duplicates manually
        first_match = matches[0]
        # Remove all matches from content, then re-insert first_match at its position
        # Actually simpler: replace all matches with a placeholder, then put the first one back.
        parts = []
        last_end = 0
        for i, match in enumerate(matches):
            parts.append(content[last_end:match.start()])
            if i == 0:
                parts.append(match.group(0))
            last_end = match.end()
        parts.append(content[last_end:])
        content = ''.join(parts)

# Fix trayIconActivated
# Ensure it doesn't have 'static' and that it uses 'this->' correctly if needed
# Actually, if it's a member function, it shouldn't need 'this->' unless there's a name conflict.
# But let's leave 'this->' as it might help the compiler if there's confusion.
# The error "static member functions do not have 'this' pointers" strongly suggests the compiler
# thinks 'trayIconActivated' IS static.
# Let's check the header again... oh wait!
# I see what I did in the header turn. 
# I added it to private slots.

# Let's check for any other 'trayIconActivated' bodies.
pattern = re.compile(r'void\s+MainWindow::trayIconActivated\s*\(\s*QSystemTrayIcon::ActivationReason\s+reason\s*\)\s*\{', re.MULTILINE)
matches = list(pattern.finditer(content))
if len(matches) > 1:
    print(f"Found {len(matches)} definitions of trayIconActivated, deduping...")
    # Deduplicate similarly
    parts = []
    last_end = 0
    for i, match in enumerate(matches):
        # find closing brace
        brace_count = 0
        end_pos = match.end()
        for j in range(match.end() - 1, len(content)):
            if content[j] == '{': brace_count += 1
            elif content[j] == '}': 
                brace_count -= 1
                if brace_count == 0:
                    end_pos = j + 1
                    break
        
        parts.append(content[last_end:match.start()])
        if i == 0:
            parts.append(content[match.start():end_pos])
        last_end = end_pos
    parts.append(content[last_end:])
    content = ''.join(parts)

# Final check for 'isVisible', 'hide', etc calling in trayIconActivated
# Maybe use a simpler body for trayIconActivated to avoid any weirdness
new_body = """void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (this->isVisible()) {
            this->hide();
        } else {
            this->showNormal();
            this->activateWindow();
        }
    }
}"""

# Replace the body
content = re.sub(r'void\s+MainWindow::trayIconActivated\s*\(\s*QSystemTrayIcon::ActivationReason\s+reason\s*\)\s*\{.*?\}', new_body, content, flags=re.DOTALL)

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content)

print(f"Cleaned up and deduped {file_path}")
