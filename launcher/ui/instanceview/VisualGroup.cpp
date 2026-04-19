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

#include "VisualGroup.h"

#include <QApplication>
#include <QDebug>
#include <QModelIndex>
#include <QPainter>
#include <QtMath>

#include "InstanceView.h"

VisualGroup::VisualGroup(const QString &text, InstanceView *view)
    : view(view), text(text), collapsed(false) {}

VisualGroup::VisualGroup(const VisualGroup *other)
    : view(other->view), text(other->text), collapsed(other->collapsed) {}

void VisualGroup::update() {
  auto temp_items = items();
  auto itemsPerRow = view->itemsPerRow();

  int numRows = qMax(1, qCeil((qreal)temp_items.size() / (qreal)itemsPerRow));
  rows = QVector<VisualRow>(numRows);

  int maxRowHeight = 0;
  int positionInRow = 0;
  int currentRow = 0;
  int offsetFromTop = 0;
  for (auto item : temp_items) {
    if (positionInRow == itemsPerRow) {
      rows[currentRow].height = maxRowHeight;
      rows[currentRow].top = offsetFromTop;
      currentRow++;
      offsetFromTop += maxRowHeight + 5;
      positionInRow = 0;
      maxRowHeight = 0;
    }
    auto itemHeight =
        view->itemDelegate()->sizeHint(view->viewOptions(), item).height();
    if (itemHeight > maxRowHeight) {
      maxRowHeight = itemHeight;
    }
    rows[currentRow].items.append(item);
    positionInRow++;
  }
  rows[currentRow].height = maxRowHeight;
  rows[currentRow].top = offsetFromTop;
}

QPair<int, int> VisualGroup::positionOf(const QModelIndex &index) const {
  int y = 0;
  for (auto &row : rows) {
    for (auto x = 0; x < row.items.size(); x++) {
      if (row.items[x] == index) {
        return qMakePair(x, y);
      }
    }
    y++;
  }
  qWarning() << "Item" << index.row() << index.data(Qt::DisplayRole).toString()
             << "not found in visual group" << text;
  return qMakePair(0, 0);
}

int VisualGroup::rowTopOf(const QModelIndex &index) const {
  auto position = positionOf(index);
  return rows[position.second].top;
}

int VisualGroup::rowHeightOf(const QModelIndex &index) const {
  auto position = positionOf(index);
  return rows[position.second].height;
}

VisualGroup::HitResults VisualGroup::hitScan(const QPoint &pos) const {
  VisualGroup::HitResults results = VisualGroup::NoHit;
  int y_start = verticalPosition();
  int body_start = y_start + headerHeight();
  int body_end = body_start + contentHeight() + 5; // FIXME: wtf is this 5?
  int y = pos.y();
  // int x = pos.x();
  if (y < y_start) {
    results = VisualGroup::NoHit;
  } else if (y < body_start) {
    results = VisualGroup::HeaderHit;
    int collapseSize = headerHeight() - 4;

    // the icon
    QRect iconRect =
        QRect(view->m_leftMargin + 2, 2 + y_start, collapseSize, collapseSize);
    if (iconRect.contains(pos)) {
      results |= VisualGroup::CheckboxHit;
    }
  } else if (y < body_end) {
    results |= VisualGroup::BodyHit;
  }
  return results;
}

void VisualGroup::drawHeader(QPainter *painter,
                             const QStyleOptionViewItem &option) {
  painter->setRenderHint(QPainter::Antialiasing);

  const QRect optRect = option.rect;
  QFont font(QApplication::font());
  font.setBold(true);
  const QFontMetrics fontMetrics = QFontMetrics(font);

  // Modern pill background with accent wash
  {
    QColor pillBg = option.palette.color(QPalette::Highlight);
    pillBg.setAlpha(20);
    painter->save();
    painter->setBrush(pillBg);
    painter->setPen(Qt::NoPen);

    QRect pillRect = optRect;
    pillRect.setHeight(fontMetrics.height() + 12);
    pillRect.adjust(4, 2, -4, 0);
    painter->drawRoundedRect(pillRect, 6, 6);
    painter->restore();
  }

  // Chevron indicator (▶ collapsed, ▼ expanded)
  {
    painter->save();
    painter->setFont(font);
    QColor chevronColor = option.palette.color(QPalette::Highlight);
    chevronColor.setAlpha(180);
    painter->setPen(chevronColor);
    painter->setBrush(chevronColor);

    int chevronSize = fontMetrics.height() - 4;
    int chevronX = optRect.left() + 12;
    int chevronY = optRect.top() + 8;

    // Draw a triangle
    QPolygonF chevron;
    if (collapsed) {
      // Right-pointing triangle ▶
      chevron << QPointF(chevronX, chevronY)
              << QPointF(chevronX + chevronSize * 0.7,
                         chevronY + chevronSize / 2.0)
              << QPointF(chevronX, chevronY + chevronSize);
    } else {
      // Down-pointing triangle ▼
      chevron << QPointF(chevronX, chevronY)
              << QPointF(chevronX + chevronSize, chevronY)
              << QPointF(chevronX + chevronSize / 2.0,
                         chevronY + chevronSize * 0.7);
    }
    painter->drawPolygon(chevron);
    painter->restore();
  }

  // Group text
  {
    QRect textRect(option.rect);
    textRect.setTop(textRect.top() + 7);
    textRect.setLeft(textRect.left() + 12 + fontMetrics.height() + 4);
    textRect.setHeight(fontMetrics.height());
    textRect.setRight(textRect.right() - 7);

    painter->save();
    painter->setFont(font);
    QColor penColor(option.palette.text().color());
    penColor.setAlphaF(0.75f);
    painter->setPen(penColor);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    painter->restore();
  }
}

int VisualGroup::totalHeight() const {
  return headerHeight() + 5 + contentHeight(); // FIXME: wtf is that '5'?
}

int VisualGroup::headerHeight() const {
  QFont font(QApplication::font());
  font.setBold(true);
  QFontMetrics fontMetrics(font);

  const int height = fontMetrics.height() + 1 /* 1 pixel-width gradient */
                     + 11 /* top and bottom separation */;
  return height;
  /*
  int raw = view->viewport()->fontMetrics().height() + 4;
  // add english. maybe. depends on font height.
  if (raw % 2 == 0)
      raw++;
  return std::min(raw, 25);
  */
}

int VisualGroup::contentHeight() const {
  if (collapsed) {
    return 0;
  }
  auto last = rows[numRows() - 1];
  return last.top + last.height;
}

int VisualGroup::numRows() const { return rows.size(); }

int VisualGroup::verticalPosition() const { return m_verticalPosition; }

QList<QModelIndex> VisualGroup::items() const {
  QList<QModelIndex> indices;
  for (int i = 0; i < view->model()->rowCount(); ++i) {
    const QModelIndex index = view->model()->index(i, 0);
    if (index.data(InstanceViewRoles::GroupRole).toString() == text) {
      indices.append(index);
    }
  }
  return indices;
}
