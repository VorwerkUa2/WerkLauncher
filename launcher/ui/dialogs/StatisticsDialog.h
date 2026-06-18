#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

class StatisticsDialog : public QDialog {
    Q_OBJECT
public:
    explicit StatisticsDialog(QWidget *parent = nullptr);
    ~StatisticsDialog() override;

private:
    void populate();

    QVBoxLayout *m_layout;
    QLabel *m_totalTimeLabel;
    QTableWidget *m_table;
    QPushButton *m_closeButton;
};
