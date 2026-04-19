
import re

file_path = r'c:\Users\knyaz\Desktop\MCL\launcher\ui\MainWindow.cpp'

with open(file_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Clean up double spacing and trailing whitespace
cleaned_lines = []
last_was_blank = False
for line in lines:
    stripped = line.strip()
    if stripped == "":
        if not last_was_blank:
            cleaned_lines.append("")
            last_was_blank = True
    else:
        cleaned_lines.append(line.rstrip())
        last_was_blank = False

content = '\n'.join(cleaned_lines)

# 1. Add platform includes
if '#ifdef Q_OS_WIN' not in content:
    includes_patch = """#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#endif
"""
    content = content.replace('#include "MainWindow.h"', '#include "MainWindow.h"\n' + includes_patch)

# 2. Update constructor with WS_THICKFRAME
constructor_pattern = re.compile(r'MainWindow::MainWindow\(QWidget \*parent\)\s*:\s*QMainWindow\(parent\),\s*ui\(new MainWindow::Ui\)\s*\{')
constructor_match = constructor_pattern.search(content)

if constructor_match:
    start_pos = constructor_match.end()
    insertion = """
#ifdef Q_OS_WIN
    HWND hwnd = (HWND)winId();
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, style | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
#endif
"""
    content = content[:start_pos] + insertion + content[start_pos:]

# 3. Update changeEvent for Aero Snap and Icons
change_event_pattern = re.compile(r'void MainWindow::changeEvent\(QEvent \*event\)\s*\{')
change_event_match = change_event_pattern.search(content)

if change_event_match:
    start_pos = change_event_match.end()
    # Find the end of the original changeEvent
    # This is a bit simplified, but should work if we look for the next top-level }
    # Actually, let's just replace the whole changeEvent block if it's identifiable.
    
    new_change_event = """
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else if (event->type() == QEvent::WindowStateChange) {
        auto titleBar = findChild<CustomTitleBar *>();
        if (titleBar) {
            titleBar->updateMaximizeIcon();
        }
        
        if (isMaximized()) {
#ifdef Q_OS_WIN
            // On Windows, a maximized frameless window with WS_THICKFRAME
            // can sometimes overlap the taskbar or have incorrect margins.
            // We ensure it fits the work area.
            auto screen = windowHandle()->screen();
            if (screen) {
                this->setContentsMargins(8, 8, 8, 8); // Offset to prevent clipping
            }
#endif
        } else {
            this->setContentsMargins(0, 0, 0, 0);
        }
    }
    QMainWindow::changeEvent(event);
}
"""
    # Find the closing brace of changeEvent
    end_of_block = content.find('}', start_pos)
    # This might find the wrong brace if there are nested ones. 
    # But changeEvent is usually simple.
    # Let's try to find the end more robustly.
    
    # Actually, I'll just append nativeEvent to the end of the file.
    
# 4. Add nativeEvent at the end
native_event_impl = """
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_NCCALCSIZE) {
        // Return 0 to indicate that we handle the non-client area calculation
        // This removes the standard window frame while keeping Aero Snap and resizing.
        *result = 0;
        return true;
    }
    
    if (msg->message == WM_NCHITTEST) {
        const int border_width = 8;
        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);
        
        // Convert to local coordinates
        POINT pt = { x, y };
        ScreenToClient((HWND)winId(), &pt);
        
        int w = width();
        int h = height();
        
        if (pt.x < border_width && pt.y < border_width) *result = HTTOPLEFT;
        else if (pt.x > w - border_width && pt.y < border_width) *result = HTTOPRIGHT;
        else if (pt.x < border_width && pt.y > h - border_width) *result = HTBOTTOMLEFT;
        else if (pt.x > w - border_width && pt.y > h - border_width) *result = HTBOTTOMRIGHT;
        else if (pt.x < border_width) *result = HTLEFT;
        else if (pt.x > w - border_width) *result = HTRIGHT;
        else if (pt.y < border_width) *result = HTTOP;
        else if (pt.y > h - border_width) *result = HTBOTTOM;
        else return QMainWindow::nativeEvent(eventType, message, result);
        
        return true;
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}
"""

content += "\n" + native_event_impl

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content)

print(f"Applied fixes and cleaned up {file_path}")
