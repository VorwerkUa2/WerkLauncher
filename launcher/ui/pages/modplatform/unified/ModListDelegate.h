#pragma once

#include <QStyledItemDelegate>

namespace Unified {

class ModListDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ModListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

} // namespace Unified
