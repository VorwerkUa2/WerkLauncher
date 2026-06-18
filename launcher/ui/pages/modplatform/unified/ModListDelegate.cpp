#include "ModListDelegate.h"
#include "UnifiedModData.h"
#include <QPainter>
#include <QApplication>

namespace Unified {

ModListDelegate::ModListDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QSize ModListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), 80); // Fixed height for list items
}

void ModListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
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
        bgColor.setAlpha(30);
    } else if (option.state & QStyle::State_MouseOver) {
        bgColor = option.palette.color(QPalette::AlternateBase);
        borderColor = option.palette.color(QPalette::Mid);
        borderColor.setAlpha(100);
    }
    
    painter->setBrush(bgColor);
    if (borderColor == Qt::transparent) {
        painter->setPen(Qt::NoPen);
    } else {
        painter->setPen(QPen(borderColor, 1));
    }
    
    // Draw a small border/margin between items
    QRect cardRect = rect.adjusted(2, 2, -2, -2);
    painter->drawRoundedRect(cardRect, 6, 6);

    // Margin inside card
    int m = 8;
    QRect contentRect = cardRect.adjusted(m, m, -m, -m);

    // Get Data
    QVariant dataVar = index.data(Qt::UserRole);
    if (!dataVar.canConvert<UnifiedModData>()) {
        painter->restore();
        return;
    }
    UnifiedModData mod = dataVar.value<UnifiedModData>();

    // Draw Icon (48x48) on the left
    int iconSize = 48;
    QRect iconRect(contentRect.x(), contentRect.y() + (contentRect.height() - iconSize) / 2, iconSize, iconSize);
    
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        icon.paint(painter, iconRect, Qt::AlignCenter);
    } else {
        painter->setBrush(option.palette.color(QPalette::AlternateBase));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(iconRect, 4, 4);
    }

    // Texts on the right of the icon
    int textX = iconRect.right() + 10;
    
    // Mod Name
    QRect nameRect(textX, contentRect.top(), contentRect.right() - textX - 5, 20);
    QFont nameFont = option.font;
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 1);
    painter->setFont(nameFont);
    
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Text));
    } else {
        painter->setPen(option.palette.color(QPalette::Text));
    }
    
    QString elidedName = painter->fontMetrics().elidedText(mod.name, Qt::ElideRight, nameRect.width());
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    // Author & Downloads
    QRect authorRect(textX, nameRect.bottom() + 2, contentRect.width() - textX + contentRect.left(), 16);
    QFont infoFont = option.font;
    infoFont.setPointSize(infoFont.pointSize() - 1);
    painter->setFont(infoFont);
    
    QColor grayColor = option.palette.color(QPalette::Text);
    grayColor.setAlpha(150);
    painter->setPen(grayColor);

    QString dlStr;
    if (mod.downloadCount >= 1000000) {
        dlStr = QString::number(mod.downloadCount / 1000000.0, 'f', 1) + "M DLs";
    } else if (mod.downloadCount >= 1000) {
        dlStr = QString::number(mod.downloadCount / 1000.0, 'f', 1) + "K DLs";
    } else {
        dlStr = QString::number(mod.downloadCount) + " DLs";
    }
    
    QString authorStr = QString("by %1 • %2").arg(mod.author).arg(dlStr);
    painter->drawText(authorRect, Qt::AlignLeft | Qt::AlignVCenter, authorStr);

    // Summary
    QRect summaryRect(textX, authorRect.bottom() + 2, contentRect.width() - textX + contentRect.left(), 30);
    painter->setFont(option.font); // normal font
    QString elidedSummary = painter->fontMetrics().elidedText(mod.summary, Qt::ElideRight, summaryRect.width());
    painter->drawText(summaryRect, Qt::AlignLeft | Qt::AlignTop, elidedSummary);

    painter->restore();
}

} // namespace Unified
