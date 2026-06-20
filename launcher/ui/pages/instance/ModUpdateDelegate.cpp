#include "ModUpdateDelegate.h"
#include "minecraft/mod/ModFolderModel.h"
#include "minecraft/mod/Mod.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QModelIndex>

ModUpdateDelegate::ModUpdateDelegate(ModFolderModel *model, QObject *parent)
    : QStyledItemDelegate(parent), m_model(model)
{
}

QRect ModUpdateDelegate::updateButtonRect(const QStyleOptionViewItem &option) const
{
    int btnW = 80;
    int btnH = 22;
    int x = option.rect.right() - btnW - 4;
    int y = option.rect.center().y() - btnH / 2;
    return QRect(x, y, btnW, btnH);
}

void ModUpdateDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (index.column() != ModFolderModel::NameColumn)
        return;

    // Map proxy index to source row
    int sourceRow = index.row();
    auto *proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
    if (proxy) {
        sourceRow = proxy->mapToSource(index).row();
    }

    if (sourceRow < 0 || sourceRow >= m_model->rowCount(QModelIndex()))
        return;

    const Mod &mod = m_model->at(sourceRow);
    if (!mod.hasUpdate())
        return;

    painter->save();

    QRect btnRect = updateButtonRect(option);

    // Draw update button
    QColor btnColor(0xE6, 0x9D, 0x00); // orange/gold accent
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(btnColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(btnRect, 4, 4);

    painter->setPen(Qt::black);
    QFont font = option.font;
    font.setPointSize(8);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(btnRect, Qt::AlignCenter, tr("Оновити"));

    painter->restore();
}

QSize ModUpdateDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    if (s.height() < 28)
        s.setHeight(28);
    return s;
}

bool ModUpdateDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index)
{
    if (index.column() != ModFolderModel::NameColumn)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    if (event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent*>(event);

        int sourceRow = index.row();
        auto *proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
        if (proxy) {
            sourceRow = proxy->mapToSource(index).row();
        }

        if (sourceRow < 0 || sourceRow >= m_model->rowCount(QModelIndex()))
            return false;

        const Mod &mod = m_model->at(sourceRow);
        if (!mod.hasUpdate())
            return false;

        QRect btnRect = updateButtonRect(option);
        if (btnRect.contains(mouseEvent->pos())) {
            emit updateClicked(sourceRow);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
