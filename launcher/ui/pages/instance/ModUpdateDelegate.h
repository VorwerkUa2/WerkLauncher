#pragma once

#include <QStyledItemDelegate>
#include <QObject>

class ModFolderModel;

class ModUpdateDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ModUpdateDelegate(ModFolderModel *model, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void updateClicked(int sourceRow);
    void changeVersionClicked(int sourceRow);

private:
    QRect updateButtonRect(const QStyleOptionViewItem &option) const;
    ModFolderModel *m_model;
};
