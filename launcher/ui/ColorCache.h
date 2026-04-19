#pragma once
#include <MessageLevel.h>
#include <QMap>
#include <QtGui/QColor>
#include <rainbow.h>


class ColorCache {
public:
  ColorCache(QColor front, QColor back, qreal bias) {
    m_front = front;
    m_back = back;
    m_bias = bias;
  };

  void addColor(int key, QColor color) {
    m_colors[key] = {color, blend(color), blendBackground(color)};
  }

  void setForeground(QColor front) {
    if (m_front != front) {
      m_front = front;
      recolorAll();
    }
  }

  void setBackground(QColor back) {
    if (m_back != back) {
      m_back = back;
      recolorAll();
    }
  }

  QColor getFront(int key) {
    auto iter = m_colors.find(key);
    if (iter == m_colors.end()) {
      return QColor();
    }
    return (*iter).front;
  }

  QColor getBack(int key) {
    auto iter = m_colors.find(key);
    if (iter == m_colors.end()) {
      return QColor();
    }
    return (*iter).back;
  }

  /**
   * Blend the color with the front color, adapting to the back color
   */
  QColor blend(QColor color);

  /**
   * Blend the color with the back color
   */
  QColor blendBackground(QColor color);

protected:
  void recolorAll();

protected:
  struct ColorEntry {
    QColor original;
    QColor front;
    QColor back;
  };

protected:
  qreal m_bias;
  QColor m_front;
  QColor m_back;
  QMap<int, ColorEntry> m_colors;
};

class LogColorCache : public ColorCache {
public:
  LogColorCache(QColor front, QColor back) : ColorCache(front, back, 1.0) {
    bool isDark = Rainbow::luma(front) > Rainbow::luma(back);
    if (isDark) {
      addColorExact((int)MessageLevel::Launcher,
                    QColor("#df80ff")); // Light purple
      addColorExact((int)MessageLevel::Debug, QColor("#80ff80")); // Light green
      addColorExact((int)MessageLevel::Warning,
                    QColor("#ffbf66")); // Light orange
      addColorExact((int)MessageLevel::Error, QColor("#ff8080")); // Light red
      addColorExact((int)MessageLevel::Fatal, QColor("#ff8080"));
      addColorExact((int)MessageLevel::Message, front);
    } else {
      addColorExact((int)MessageLevel::Launcher,
                    QColor("#800080"));                           // Dark purple
      addColorExact((int)MessageLevel::Debug, QColor("#008000")); // Dark green
      addColorExact((int)MessageLevel::Warning,
                    QColor("#cc6600"));                           // Dark orange
      addColorExact((int)MessageLevel::Error, QColor("#cc0000")); // Dark red
      addColorExact((int)MessageLevel::Fatal, QColor("#cc0000"));
      addColorExact((int)MessageLevel::Message, front);
    }
  }

  void addColorExact(int key, QColor fgColor) {
    m_colors[key] = {fgColor, fgColor, QColor(Qt::transparent)};
  }

  QColor getFront(MessageLevel::Enum level) {
    if (!m_colors.contains((int)level)) {
      return ColorCache::getFront((int)MessageLevel::Message);
    }
    return ColorCache::getFront((int)level);
  }

  QColor getBack(MessageLevel::Enum level) {
    if (level == MessageLevel::Fatal) {
      return QColor(Qt::black); // or dark grey? Let's leave it high contrast
    }
    return QColor(Qt::transparent);
  }
};
