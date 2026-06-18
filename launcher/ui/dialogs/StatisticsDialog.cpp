#include "StatisticsDialog.h"

#include <QHeaderView>
#include <QDateTime>
#include "Application.h"
#include "InstanceList.h"
#include "BaseInstance.h"
#include "MMCTime.h"
#include "icons/IconList.h"
#include "ui/widgets/CustomTitleBar.h"

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Playtime Statistics"));
    setMinimumSize(400, 300);
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    auto titleBar = new CustomTitleBar(this);
    titleBar->setTitle(tr("Playtime Statistics"));
    m_layout->addWidget(titleBar);

    auto contentWidget = new QWidget(this);
    auto contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    m_layout->addWidget(contentWidget);

    m_totalTimeLabel = new QLabel(this);
    QFont font = m_totalTimeLabel->font();
    font.setPointSize(font.pointSize() + 2);
    font.setBold(true);
    m_totalTimeLabel->setFont(font);
    m_totalTimeLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(m_totalTimeLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({tr("Instance"), tr("Playtime")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    contentLayout->addWidget(m_table);

    m_closeButton = new QPushButton(tr("Close"), this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    contentLayout->addWidget(m_closeButton, 0, Qt::AlignRight);

    populate();
}

StatisticsDialog::~StatisticsDialog() {
}

void StatisticsDialog::populate() {
    int totalTimePlayed = APPLICATION->instances()->getTotalPlayTime();
    QString totalTimeStr = totalTimePlayed > 0 ? Time::prettifyDuration(totalTimePlayed) : "0s";
    m_totalTimeLabel->setText(tr("Total Playtime: %1").arg(totalTimeStr));

    int count = APPLICATION->instances()->count();
    m_table->setRowCount(0);

    for (int i = 0; i < count; ++i) {
        auto inst = APPLICATION->instances()->at(i);
        if (!inst) continue;

        int64_t timePlayed = inst->totalTimePlayed();
        if (timePlayed <= 0) continue;

        int row = m_table->rowCount();
        m_table->insertRow(row);

        QTableWidgetItem *nameItem = new QTableWidgetItem(inst->name());
        nameItem->setIcon(APPLICATION->icons()->getIcon(inst->iconKey()));
        m_table->setItem(row, 0, nameItem);

        QTableWidgetItem *timeItem = new QTableWidgetItem(Time::prettifyDuration(timePlayed));
        timeItem->setData(Qt::UserRole, QVariant::fromValue(timePlayed));
        m_table->setItem(row, 1, timeItem);
    }
    
    // Sort by playtime by default (descending)
    m_table->sortItems(1, Qt::DescendingOrder);
}
