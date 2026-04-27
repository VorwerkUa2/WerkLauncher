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

  // Subtle drop shadow for depth on all items
  QColor shadowColor(0, 0, 0, 25);
  painter->setBrush(shadowColor);
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(rect.adjusted(3, 3, -1, -1), 8, 8);

  if ((option.state & QStyle::State_Selected)) {
    QColor highlightColor = option.palette.color(QPalette::Highlight);
    painter->setBrush(highlightColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 8, 8);
  } else if (option.state & QStyle::State_MouseOver) {
    QColor hoverBg = option.palette.color(QPalette::Highlight);
    hoverBg.setAlpha(20);
    QColor hoverBorder = option.palette.color(QPalette::Highlight);
    hoverBorder.setAlpha(70);
    painter->setBrush(hoverBg);
    painter->setPen(QPen(hoverBorder, 1.0));
    painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 8, 8);

    // Softer, wider outer glow for hover
    QColor glowColor = option.palette.color(QPalette::Highlight);
    glowColor.setAlpha(25);
    painter->setPen(QPen(glowColor, 3));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rect.adjusted(0, 0, 0, 0), 10, 10);
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

  // const int iconSize =  style->pixelMetric(QStyle::PM_IconViewIconSize);
  static const int iconSize = 52;
  QRect iconbox = opt.rect;
  const int textMargin =
      style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, opt.widget) + 1;
  QRect textRect = opt.rect;
  QRect textHighlightRect = textRect;
  // clip the decoration on top, remove width padding
  textRect.adjust(textMargin, iconSize + textMargin + 5, -textMargin, 0);

  textHighlightRect.adjust(0, iconSize + 5, 0, 0);

  // draw background
  {
    // FIXME: unused
    // QSize textSize = viewItemTextSize ( &opt );
    drawSelectionRect(painter, opt, textHighlightRect);
    /*
    QPalette::ColorGroup cg;
    QStyleOptionViewItem opt2(opt);

    if ((opt.widget && opt.widget->isEnabled()) || (opt.state &
    QStyle::State_Enabled))
    {
        if (!(opt.state & QStyle::State_Active))
            cg = QPalette::Inactive;
        else
            cg = QPalette::Normal;
    }
    else
    {
        cg = QPalette::Disabled;
    }
    */
    /*
    opt2.palette.setCurrentColorGroup(cg);

    // fill in background, if any


    if (opt.backgroundBrush.style() != Qt::NoBrush)
    {
        QPointF oldBO = painter->brushOrigin();
        painter->setBrushOrigin(opt.rect.topLeft());
        painter->fillRect(opt.rect, opt.backgroundBrush);
        painter->setBrushOrigin(oldBO);
    }

    drawSelectionRect(painter, opt2, textHighlightRect);
    */

    /*
    if (opt.showDecorationSelected)
    {
        drawSelectionRect(painter, opt2, opt.rect);
        drawFocusRect(painter, opt2, opt.rect);
        // painter->fillRect ( opt.rect, opt.palette.brush ( cg,
    QPalette::Highlight ) );
    }
    else
    {

        // if ( opt.state & QStyle::State_Selected )
        {
            // QRect textRect = subElementRect ( QStyle::SE_ItemViewItemText,
    opt,
            // opt.widget );
            // painter->fillRect ( textHighlightRect, opt.palette.brush ( cg,
            // QPalette::Highlight ) );
            drawSelectionRect(painter, opt2, textHighlightRect);
            drawFocusRect(painter, opt2, textHighlightRect);
        }
    }
    */
  }

  // icon mode and state, also used for badges
  QIcon::Mode mode = QIcon::Normal;
  if (!(opt.state & QStyle::State_Enabled))
    mode = QIcon::Disabled;
  else if (opt.state & QStyle::State_Selected)
    mode = QIcon::Selected;
  QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

  // draw the icon
  {
    iconbox.setHeight(iconSize);

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
  if (opt.state & QStyle::State_MouseOver) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    auto drawButton = [&](const QRect &r, const QString &iconName,
                          const QColor &baseColor) {
      bool hovered = r.contains(opt.widget->mapFromGlobal(QCursor::pos()));
      QColor btnColor = baseColor;
      btnColor.setAlpha(hovered ? 230 : 150);

      painter->setBrush(btnColor);
      painter->setPen(Qt::NoPen);
      painter->drawEllipse(r);

      auto icon = XdgIcon::fromTheme(iconName);
      icon.paint(painter, r.adjusted(4, 4, -4, -4), Qt::AlignCenter,
                 QIcon::Normal);
    };

    // Play button (bottom left of icon)
    QRect playRect(iconbox.left() + 4, iconbox.bottom() - 24, 24, 24);
    drawButton(playRect, "media-playback-start", QColor(46, 204, 113)); // Green

    // Config button (bottom right of icon)
    QRect configRect(iconbox.right() - 28, iconbox.bottom() - 24, 24, 24);
    drawButton(configRect, "configure", QColor(52, 152, 219)); // Blue

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
  nameFont.setWeight(QFont::DemiBold);
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
  static const int baseIconSize = 52;
  int height = baseIconSize + textMargin * 2 + 5;
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
      static const int iconSize = 52;
      QRect iconbox = option.rect;
      iconbox.setHeight(iconSize);

      // Play button (bottom left of icon)
      QRect playRect(iconbox.left() + 4, iconbox.bottom() - 24, 24, 24);
      // Config button (bottom right of icon)
      QRect configRect(iconbox.right() - 28, iconbox.bottom() - 24, 24, 24);

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
