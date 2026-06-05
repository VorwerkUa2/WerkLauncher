/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "InstanceDelegate.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QTextLayout>
#include <QTextOption>
#include <QtMath>

#include "Application.h"
#include "BaseInstance.h"
#include "InstanceList.h"
#include "InstanceView.h"
#include "MMCTime.h"
#include <QDateTime>
#include <QTextEdit>
#include <xdgicon.h>

// Origin: Qt
static void viewItemTextLayout(QTextLayout &textLayout, int lineWidth,
                               qreal &height, qreal &widthUsed) {
  height = 0;
  widthUsed = 0;
  textLayout.beginLayout();
  QString str = textLayout.text();
  while (true) {
    QTextLine line = textLayout.createLine();
    if (!line.isValid())
      break;
    if (line.textLength() == 0)
      break;
    line.setLineWidth(lineWidth);
    line.setPosition(QPointF(0, height));
    height += line.height();
    widthUsed = qMax(widthUsed, line.naturalTextWidth());
  }
  textLayout.endLayout();
}

ListViewDelegate::ListViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void drawSelectionRect(QPainter *painter, const QStyleOptionViewItem &option,
                       const QRect &rect) {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // Modern card background
  QColor cardBg = option.palette.color(QPalette::Base);
  if (cardBg.lightness() < 128) {
      cardBg = QColor(255, 255, 255, 10); // Slight white overlay for dark theme
  } else {
      cardBg = QColor(0, 0, 0, 10); // Slight dark overlay for light theme
  }
  painter->setBrush(cardBg);
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 12, 12);

  if ((option.state & QStyle::State_Selected)) {
    QColor highlightColor = option.palette.color(QPalette::Highlight);
    highlightColor.setAlpha(50);
    painter->setBrush(highlightColor);
    painter->setPen(QPen(option.palette.color(QPalette::Highlight), 1.5));
    painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 12, 12);
  } else if (option.state & QStyle::State_MouseOver) {
    QColor glowColor(230, 126, 34); // Orange glow (#e67e22)
    QColor hoverBg = glowColor;
    hoverBg.setAlpha(20);
    
    painter->setBrush(hoverBg);
    painter->setPen(QPen(glowColor, 1.5));
    painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 12, 12);

    // Glow
    glowColor.setAlpha(40);
    painter->setPen(QPen(glowColor, 4));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), 13, 13);
  }
  painter->restore();
}

void drawFocusRect(QPainter *painter, const QStyleOptionViewItem &option,
                   const QRect &rect) {
  // We prefer not to draw focus rects for a cleaner look
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(rect);
}

void drawProgressOverlay(QPainter *painter, const QStyleOptionViewItem &option,
                         const int value, const int maximum) {
  if (maximum <= 0 || value == maximum) {
    return;
  }

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  qreal percent = (qreal)value / (qreal)maximum;
  QColor accentColor = option.palette.color(QPalette::Highlight);

  // Draw a modern progress bar at the bottom of the icon or item
  QRect progressRect = option.rect;
  progressRect.setHeight(4);
  progressRect.moveBottom(option.rect.bottom() - 2);
  progressRect.setLeft(option.rect.left() + 4);
  progressRect.setRight(option.rect.right() - 4);

  // Background of progress bar
  painter->setBrush(QColor(0, 0, 0, 100));
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(progressRect, 2, 2);

  // Foreground of progress bar
  progressRect.setWidth(progressRect.width() * percent);
  painter->setBrush(accentColor);
  painter->drawRoundedRect(progressRect, 2, 2);

  painter->restore();
}

void drawBadges(QPainter *painter, const QStyleOptionViewItem &option,
                BaseInstance *instance, QIcon::Mode mode, QIcon::State state) {
  static QMap<QString, QIcon> badgeCache;
  QList<QString> pixmaps;
  if (instance->isRunning()) {
    pixmaps.append("status-running");
  } else if (instance->hasCrashed() || instance->hasVersionBroken()) {
    pixmaps.append("status-bad");
  }
  if (instance->hasUpdateAvailable()) {
    pixmaps.append("checkupdate");
  }

  static const int itemSide = 24;
  static const int spacing = 1;
  const int itemsPerRow = qMax(1, qFloor(double(option.rect.width() + spacing) /
                                         double(itemSide + spacing)));
  const int rows = qCeil((double)pixmaps.size() / (double)itemsPerRow);
  QListIterator<QString> it(pixmaps);
  painter->translate(option.rect.topLeft());
  for (int y = 0; y < rows; ++y) {
    for (int x = 0; x < itemsPerRow; ++x) {
      if (!it.hasNext()) {
        return;
      }
      const QString iconName = it.next();
      if (!badgeCache.contains(iconName)) {
        badgeCache.insert(iconName, XdgIcon::fromTheme(iconName));
      }
      auto icon = badgeCache.value(iconName);
      // itemSide
      QRect badgeRect(option.rect.width() - x * itemSide +
                          qMax(x - 1, 0) * spacing - itemSide,
                      y * itemSide + qMax(y - 1, 0) * spacing, itemSide,
                      itemSide);
      icon.paint(painter, badgeRect, Qt::AlignCenter, mode, state);
    }
  }
  painter->translate(-option.rect.topLeft());
}

static QSize viewItemTextSize(const QStyleOptionViewItem *option) {
  QStyle *style =
      option->widget ? option->widget->style() : QApplication::style();
  QTextOption textOption;
  textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  QTextLayout textLayout;
  textLayout.setTextOption(textOption);
  textLayout.setFont(option->font);
  textLayout.setText(option->text);
  const int textMargin =
      style->pixelMetric(QStyle::PM_FocusFrameHMargin, option, option->widget) +
      1;
  QRect bounds(0, 0, 100 - 2 * textMargin, 600);
  qreal height = 0, widthUsed = 0;
  viewItemTextLayout(textLayout, bounds.width(), height, widthUsed);
  const QSize size(qCeil(widthUsed), qCeil(height));
  return QSize(size.width() + 2 * textMargin, size.height());
}

void ListViewDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  painter->save();
  painter->setClipRect(opt.rect);

  opt.features |= QStyleOptionViewItem::WrapText;
  opt.text = index.data().toString();
  opt.textElideMode = Qt::ElideRight;
  opt.displayAlignment = Qt::AlignTop | Qt::AlignHCenter;

  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

  static const int iconSize = 64;
  QRect iconbox = opt.rect;
  iconbox.setHeight(iconSize);
  iconbox.moveTop(opt.rect.top() + 12); // Move icon down slightly

  const int textMargin =
      style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, opt.widget) + 1;
  QRect textRect = opt.rect;
  QRect textHighlightRect = textRect;
  // clip the decoration on top, remove width padding. Text starts below the moved icon.
  textRect.adjust(textMargin, iconSize + textMargin + 15, -textMargin, 0);

  textHighlightRect.adjust(0, iconSize + 15, 0, 0);

  QRect cardRect = opt.rect;
  cardRect.adjust(textMargin, 2, -textMargin, -2);
  drawSelectionRect(painter, opt, cardRect);

  // icon mode and state, also used for badges
  QIcon::Mode mode = QIcon::Normal;
  if (!(opt.state & QStyle::State_Enabled))
    mode = QIcon::Disabled;
  else if (opt.state & QStyle::State_Selected)
    mode = QIcon::Selected;
  QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

  // draw the icon
  {
    // Scale icon slightly on hover for "animation" feel
    if (opt.state & QStyle::State_MouseOver) {
      painter->save();
      painter->translate(iconbox.center());
      painter->scale(1.05, 1.05);
      painter->translate(-iconbox.center());
      opt.icon.paint(painter, iconbox, Qt::AlignCenter, mode, state);
      painter->restore();
    } else {
      opt.icon.paint(painter, iconbox, Qt::AlignCenter, mode, state);
    }
  }

  // quick actions on hover
  if ((opt.state & QStyle::State_MouseOver) && opt.widget) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int btnSize = 30;
    QRect cardRect = opt.rect;
    cardRect.adjust(textMargin, 2, -textMargin, -2);
    
    // Position buttons cleanly at the bottom corners of the card
    QRect playRect(cardRect.left() + 8, cardRect.bottom() - btnSize - 8, btnSize, btnSize);
    QRect configRect(cardRect.right() - btnSize - 8, cardRect.bottom() - btnSize - 8, btnSize, btnSize);

    auto drawAction = [&](const QRect &r, const QString &iconName, bool isHovered, bool isPressed, const QColor& accent) {
      painter->save();
      if (isPressed) {
        painter->translate(r.center());
        painter->scale(0.9, 0.9);
        painter->translate(-r.center());
      }
      
      QColor bgColor = opt.palette.color(QPalette::Window);
      bgColor.setAlpha(200);
      
      if (isHovered) {
        bgColor = accent;
        bgColor.setAlpha(230);
      }
      
      painter->setBrush(bgColor);
      // Soft border matching the text color but very transparent
      QColor borderColor = opt.palette.color(QPalette::Text);
      borderColor.setAlpha(isHovered ? 80 : 30);
      painter->setPen(QPen(borderColor, 1));
      painter->drawEllipse(r);
      
      auto icon = APPLICATION->getThemedIcon(iconName);
      QPixmap pix = icon.pixmap(r.size(), QIcon::Active, QIcon::On);
      QPainter p(&pix);
      p.setCompositionMode(QPainter::CompositionMode_SourceIn);
      QColor iconColor = isHovered ? Qt::white : opt.palette.color(QPalette::Text);
      p.fillRect(pix.rect(), iconColor);
      p.end();
      painter->drawPixmap(r.adjusted(6, 6, -6, -6), pix);
      painter->restore();
    };

    QPoint mousePos = opt.widget->mapFromGlobal(QCursor::pos());
    bool playHovered = playRect.contains(mousePos);
    bool configHovered = configRect.contains(mousePos);
    bool isPressed = opt.state & QStyle::State_Sunken;

    // Theme-aware soft colors for hover
    QColor playAccent = QColor(46, 204, 113); // Soft green
    QColor configAccent = opt.palette.color(QPalette::Highlight); // Theme accent

    drawAction(playRect, "status-running", playHovered, playHovered && isPressed, playAccent);
    drawAction(configRect, "settings", configHovered, configHovered && isPressed, configAccent);

    painter->restore();
  }
  // set the text colors
  QPalette::ColorGroup cg =
      opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
  if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
    cg = QPalette::Inactive;
  if (opt.state & QStyle::State_Selected) {
    painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
  } else {
    painter->setPen(opt.palette.color(cg, QPalette::Text));
  }

  // draw the text
  QTextOption textOption;
  textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  textOption.setTextDirection(opt.direction);
  textOption.setAlignment(
      QStyle::visualAlignment(opt.direction, opt.displayAlignment));
  QTextLayout textLayout;
  textLayout.setTextOption(textOption);
  QFont nameFont = opt.font;
  nameFont.setWeight(QFont::Medium);
  if (nameFont.pointSize() > 0) {
      nameFont.setPointSize(nameFont.pointSize() + 1);
  }
  textLayout.setFont(nameFont);
  textLayout.setText(opt.text);

  qreal width, height;
  viewItemTextLayout(textLayout, textRect.width(), height, width);

  const int lineCount = textLayout.lineCount();

  const QRect layoutRect =
      QStyle::alignedRect(opt.direction, opt.displayAlignment,
                          QSize(textRect.width(), int(height)), textRect);
  const QPointF position = layoutRect.topLeft();
  for (int i = 0; i < lineCount; ++i) {
    const QTextLine line = textLayout.lineAt(i);
    line.draw(painter, position);
  }

  int currentY = layoutRect.top() + int(height);
  if (int(height) > 0) {
    currentY += 2;
  }

  // draw version description sub-text
  {
    QString versionText =
        index.data(InstanceList::VersionDescriptionRole).toString();
    if (!versionText.isEmpty()) {
      QFont subFont = opt.font;
      subFont.setPointSizeF(subFont.pointSizeF() * 0.8);
      painter->setFont(subFont);
      QColor subColor = opt.palette.color(cg, QPalette::Text);
      subColor.setAlphaF(0.55f);
      if (opt.state & QStyle::State_Selected) {
        subColor = opt.palette.color(cg, QPalette::HighlightedText);
        subColor.setAlphaF(0.7f);
      }
      painter->setPen(subColor);
      QRect drawRect(layoutRect.left(), currentY, layoutRect.width(), 600);
      painter->drawText(drawRect,
                        Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                        versionText);

      // Calculate the bottom of the drawn text to adjust the layout rect
      QRect textBoundingRect = painter->fontMetrics().boundingRect(
          drawRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
          versionText);
      currentY += textBoundingRect.height() + 2;
    }
  }

  // draw playtime and last played
  auto instance = (BaseInstance *)index.data(InstanceList::InstancePointerRole)
                      .value<void *>();
  if (instance) {
    int64_t totalTime = instance->totalTimePlayed();
    qint64 lastLaunch = instance->lastLaunch();

    QString playtimeText;
    if (totalTime > 0) {
      playtimeText = tr("Played: %1").arg(Time::prettifyDuration(totalTime));
    }

    if (lastLaunch > 0 || !playtimeText.isEmpty()) {
      QFont tinyFont = opt.font;
      tinyFont.setPointSizeF(tinyFont.pointSizeF() * 0.75);
      painter->setFont(tinyFont);
      QColor tinyColor = opt.palette.color(cg, QPalette::Text);
      tinyColor.setAlphaF(0.4f);
      if (opt.state & QStyle::State_Selected) {
        tinyColor = opt.palette.color(cg, QPalette::HighlightedText);
        tinyColor.setAlphaF(0.6f);
      }
      painter->setPen(tinyColor);

      QString text;
      if (!playtimeText.isEmpty()) {
        text = playtimeText;
      }

      if (lastLaunch > 0) {
        QString dateText = QDateTime::fromMSecsSinceEpoch(lastLaunch)
                               .date()
                               .toString(Qt::ISODate);
        if (!text.isEmpty())
          text += "\n";
        text += dateText;
      }

      if (!text.isEmpty()) {
        QRect drawRect(layoutRect.left(), currentY, layoutRect.width(), 600);
        painter->drawText(
            drawRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, text);
      }
    }
    drawBadges(painter, opt, instance, mode, state);
  }

  drawProgressOverlay(
      painter, opt, index.data(InstanceViewRoles::ProgressValueRole).toInt(),
      index.data(InstanceViewRoles::ProgressMaximumRole).toInt());

  painter->restore();
}

QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  opt.features |= QStyleOptionViewItem::WrapText;
  opt.text = index.data().toString();
  opt.textElideMode = Qt::ElideRight;
  opt.displayAlignment = Qt::AlignTop | Qt::AlignHCenter;

  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int textMargin =
      style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, opt.widget) + 1;
  static const int baseIconSize = 64;
  int height = baseIconSize + textMargin * 2 + 35; // Increased base height for icon padding and bottom buttons
  QSize szz = viewItemTextSize(&opt);
  height += szz.height();

  // Add space for version description
  QString versionText =
      index.data(InstanceList::VersionDescriptionRole).toString();
  if (!versionText.isEmpty()) {
    QFont subFont = opt.font;
    subFont.setPointSizeF(subFont.pointSizeF() * 0.8);
    QFontMetrics subFm(subFont);
    QRect maxRect(0, 0, 100 - textMargin * 2, 600);
    QRect boundRect = subFm.boundingRect(
        maxRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
        versionText);
    height += boundRect.height() + 2;
  }

  // Add space for playtime and last played
  auto instance = (BaseInstance *)index.data(InstanceList::InstancePointerRole)
                      .value<void *>();
  if (instance &&
      (instance->totalTimePlayed() > 0 || instance->lastLaunch() > 0)) {
    QFont tinyFont = opt.font;
    tinyFont.setPointSizeF(tinyFont.pointSizeF() * 0.75);
    QString playtimeText;
    int64_t totalTime = instance->totalTimePlayed();
    if (totalTime > 0) {
      playtimeText = tr("Played: %1").arg(Time::prettifyDuration(totalTime));
    }
    QString text = playtimeText;
    qint64 lastLaunch = instance->lastLaunch();
    if (lastLaunch > 0) {
      QString dateText = QDateTime::fromMSecsSinceEpoch(lastLaunch)
                             .date()
                             .toString(Qt::ISODate);
      if (!text.isEmpty())
        text += "\n";
      text += dateText;
    }
    if (!text.isEmpty()) {
      QFontMetrics tinyFm(tinyFont);
      QRect maxRect(0, 0, 100 - textMargin * 2, 600);
      QRect boundRect = tinyFm.boundingRect(
          maxRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, text);
      height += boundRect.height() + 2;
    }
  }

  // FIXME: maybe the icon items could scale and keep proportions?
  QSize sz(100, height);
  return sz;
}

class NoReturnTextEdit : public QTextEdit {
  Q_OBJECT
public:
  explicit NoReturnTextEdit(QWidget *parent) : QTextEdit(parent) {
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  }
  bool event(QEvent *event) override {
    auto eventType = event->type();
    if (eventType == QEvent::KeyPress || eventType == QEvent::KeyRelease) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      auto key = keyEvent->key();
      if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        emit editingDone();
        return true;
      }
      if (key == Qt::Key_Tab) {
        return true;
      }
    }
    return QTextEdit::event(event);
  }
signals:
  void editingDone();
};

bool ListViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) {
  if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      static const int iconSize = 64;
      QRect iconbox = option.rect;
      iconbox.setHeight(iconSize);

      int btnSize = 28;
      QRect cardRect = option.rect;
      QStyle *style = option.widget ? option.widget->style() : QApplication::style();
      const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, option.widget) + 1;
      cardRect.adjust(textMargin, 2, -textMargin, -2);
      
      QRect textRect = option.rect;
      textRect.adjust(textMargin, iconSize + textMargin + 10, -textMargin, 0);

      QRect playRect(cardRect.left() + 6, textRect.top() + 2, btnSize, btnSize);
      QRect configRect(cardRect.right() - btnSize - 6, textRect.top() + 2, btnSize, btnSize);

      if (playRect.contains(mouseEvent->pos())) {
        emit launchRequested(index);
        return true;
      } else if (configRect.contains(mouseEvent->pos())) {
        emit configRequested(index);
        return true;
      }
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ListViewDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  static const int iconSize = 48;
  QRect textRect = option.rect;
  // QStyle *style = option.widget ? option.widget->style() :
  // QApplication::style();
  textRect.adjust(0, iconSize + 5, 0, 0);
  editor->setGeometry(textRect);
}

void ListViewDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const {
  auto text = index.data(Qt::EditRole).toString();
  QTextEdit *realeditor = qobject_cast<NoReturnTextEdit *>(editor);
  realeditor->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
  realeditor->append(text);
  realeditor->selectAll();
  realeditor->document()->clearUndoRedoStacks();
}

void ListViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {
  QTextEdit *realeditor = qobject_cast<NoReturnTextEdit *>(editor);
  QString text = realeditor->toPlainText();
  text.replace(QChar('\n'), QChar(' '));
  text = text.trimmed();
  if (text.size() != 0) {
    model->setData(index, text);
  }
}

QWidget *ListViewDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  auto editor = new NoReturnTextEdit(parent);
  connect(editor, &NoReturnTextEdit::editingDone, this,
          &ListViewDelegate::editingDone);
  return editor;
}

void ListViewDelegate::editingDone() {
  NoReturnTextEdit *editor = qobject_cast<NoReturnTextEdit *>(sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

#include "InstanceDelegate.moc"
