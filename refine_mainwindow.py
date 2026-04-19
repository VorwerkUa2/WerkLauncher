
import re

file_path = r'c:\Users\knyaz\Desktop\MCL\launcher\ui\MainWindow.cpp'

with open(file_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Clean up whitespace: 
# - Strip trailing whitespace
# - Replace multiple consecutive empty lines with a single empty line
cleaned_lines = []
for line in lines:
    stripped_line = line.rstrip()
    if not stripped_line and cleaned_lines and not cleaned_lines[-1]:
        continue
    cleaned_lines.append(stripped_line)

content = '\n'.join(cleaned_lines)

# Fix changeEvent
change_event_pattern = re.compile(r'void MainWindow::changeEvent\(QEvent \*event\)\s*\{.*?\}', re.DOTALL)
new_change_event = """void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else if (event->type() == QEvent::WindowStateChange) {
        auto titleBar = findChild<CustomTitleBar *>();
        if (titleBar) {
            titleBar->updateMaximizeIcon();
        }
        
        if (isMaximized()) {
#ifdef Q_OS_WIN
            this->setContentsMargins(8, 8, 8, 8);
#endif
        } else {
            this->setContentsMargins(0, 0, 0, 0);
        }
    }
    QMainWindow::changeEvent(event);
}"""

if change_event_pattern.search(content):
    content = change_event_pattern.sub(new_change_event, content)
else:
    print("Warning: changeEvent pattern not found!")

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content)

print(f"Refined {file_path}")
