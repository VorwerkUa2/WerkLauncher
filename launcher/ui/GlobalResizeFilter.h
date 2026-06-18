#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QMainWindow>
#include <QCursor>

class GlobalResizeFilter : public QObject {
public:
    GlobalResizeFilter(QMainWindow *window) : QObject(window), m_window(window) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (!m_window || m_window->isMaximized() || m_window->isFullScreen()) {
            return false;
        }

        // IMPORTANT: Only process events for the main window and its descendants!
        // Ignore top-level dialogs (like Instance Settings) so they function normally.
        if (obj->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(obj);
            if (w->window() != m_window) return false;
        } else if (obj->inherits("QWindow")) {
            QWindow *w = static_cast<QWindow *>(obj);
            if (w != m_window->windowHandle()) return false;
        }

        if (event->type() == QEvent::MouseMove || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverEnter) {
            QPoint globalPos = QCursor::pos();
            if (updateCursor(globalPos)) {
                return true; // Consume event so child widgets don't override the cursor
            }
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                int edge = getEdge(QCursor::pos());
                if (edge != 0) {
                    // Trigger native OS resize loop. This is synchronous on Windows.
                    // It perfectly handles visual redrawing, aero snap, and minimum sizes.
                    if (m_window->windowHandle()) {
                        m_window->windowHandle()->startSystemResize(static_cast<Qt::Edges>(edge));
                    }
                    return true; // Consume event
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            // No longer strictly needed since startSystemResize is modal, but kept for safety
            m_window->unsetCursor();
        }

        return false;
    }

private:
    int getEdge(const QPoint &globalPos) {
        QRect rect = m_window->geometry();
        int x = globalPos.x() - rect.x();
        int y = globalPos.y() - rect.y();
        int w = rect.width();
        int h = rect.height();
        int border = 4;

        // "до створення інстансу змінювати розмір через верхню частину я не міг"
        // This is because the top area has the titlebar. But the user SHOULD be able 
        // to resize from the top edge. We just need to make sure the edge detection 
        // takes precedence over the titlebar.
        int edge = 0;
        if (x < border) edge |= Qt::LeftEdge;
        if (x > w - border) edge |= Qt::RightEdge;
        if (y < border) edge |= Qt::TopEdge;
        if (y > h - border) edge |= Qt::BottomEdge;

        return edge;
    }

    bool updateCursor(const QPoint &globalPos) {
        int edge = getEdge(globalPos);
        if (edge == 0) {
            if (m_cursorSet) {
                m_window->unsetCursor();
                m_cursorSet = false;
            }
            return false;
        }

        Qt::CursorShape shape = Qt::ArrowCursor;
        if ((edge & Qt::LeftEdge) && (edge & Qt::TopEdge)) shape = Qt::SizeFDiagCursor;
        else if ((edge & Qt::RightEdge) && (edge & Qt::BottomEdge)) shape = Qt::SizeFDiagCursor;
        else if ((edge & Qt::RightEdge) && (edge & Qt::TopEdge)) shape = Qt::SizeBDiagCursor;
        else if ((edge & Qt::LeftEdge) && (edge & Qt::BottomEdge)) shape = Qt::SizeBDiagCursor;
        else if (edge & Qt::LeftEdge || edge & Qt::RightEdge) shape = Qt::SizeHorCursor;
        else if (edge & Qt::TopEdge || edge & Qt::BottomEdge) shape = Qt::SizeVerCursor;

        m_window->setCursor(shape);
        m_cursorSet = true;
        return true;
    }

    QMainWindow *m_window;
    bool m_cursorSet = false;
};
