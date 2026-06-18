#include "ModpackDelegate.h"
#include <QPainter>
#include <QApplication>

ModpackDelegate::ModpackDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void ModpackDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rect = option.rect;

    // Draw background
    QColor bgColor = option.palette.color(QPalette::Base);
    QColor borderColor = Qt::transparent;

    if (option.state & QStyle::State_Selected) {
        borderColor = option.palette.color(QPalette::Highlight);
        bgColor = borderColor;
        bgColor.setAlpha(30); // light highlight background
    } else if (option.state & QStyle::State_MouseOver) {
        bgColor = option.palette.color(QPalette::AlternateBase);
        borderColor = option.palette.color(QPalette::Mid);
        borderColor.setAlpha(100);
    }
    
    painter->setBrush(bgColor);
    if (borderColor == Qt::transparent) {
        painter->setPen(Qt::NoPen);
    } else {
        painter->setPen(QPen(borderColor, 2));
    }
    painter->drawRoundedRect(rect, 8, 8);

    // Margin for content
    int m = 8;
    rect.adjust(m, m, -m, -m);

    // Get Data
    QString name = index.data(Qt::DisplayRole).toString();
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QString author = index.data(Qt::UserRole).toString();
    qulonglong downloads = index.data(Qt::UserRole + 1).toULongLong();

    // Draw Icon (64x64)
    int iconSize = 64;
    QRect iconRect(rect.x() + (rect.width() - iconSize) / 2, rect.y(), iconSize, iconSize);
    if (!icon.isNull()) {
        icon.paint(painter, iconRect, Qt::AlignCenter);
    }

    // Draw Name
    QRect nameRect(rect.x(), iconRect.bottom() + m, rect.width(), 35); // Allow 2 lines
    QFont nameFont = option.font;
    nameFont.setBold(true);
    painter->setFont(nameFont);
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Text)); // Text shouldn't be white if bg is light
    } else {
        painter->setPen(option.palette.color(QPalette::Text));
    }
    
    // Draw text with word wrap, elide if too long
    painter->drawText(nameRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, name);

    // Draw Author & Downloads
    QRect infoRect(rect.x(), nameRect.bottom() + 2, rect.width(), rect.bottom() - nameRect.bottom());
    QFont infoFont = option.font;
    infoFont.setPointSize(infoFont.pointSize() - 1); // Slightly smaller
    painter->setFont(infoFont);
    
    if (!(option.state & QStyle::State_Selected)) {
        QColor grayColor = option.palette.color(QPalette::Text);
        grayColor.setAlpha(150);
        painter->setPen(grayColor);
    }

    QString infoText;
    if (!author.isEmpty()) {
        infoText += author + "\n";
    }
    if (downloads > 0) {
        // Format downloads (e.g. 1.2M, 500K)
        QString dlStr;
        if (downloads >= 1000000) {
            dlStr = QString::number(downloads / 1000000.0, 'f', 1) + "M";
        } else if (downloads >= 1000) {
            dlStr = QString::number(downloads / 1000.0, 'f', 1) + "K";
        } else {
            dlStr = QString::number(downloads);
        }
        infoText += dlStr + " DLs";
    }

    painter->drawText(infoRect, Qt::AlignHCenter | Qt::AlignTop, infoText);

    painter->restore();
}

QSize ModpackDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
    return QSize(160, 160);
}
