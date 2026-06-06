// LinkListView.cpp

#include "LinkListView.h"
#include <QHeaderView>

LinkListView::LinkListView(QWidget *parent)
    : QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAlternatingRowColors(true);
    setSortingEnabled(true);
    verticalHeader()->hide();
    setShowGrid(false);

    connect(this, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (index.isValid())
        {
            int linkId = index.data(Qt::UserRole).toInt();
            emit linkDoubleClicked(linkId);
        }
    });
}

void LinkListView::setLinkData(QStandardItemModel *model)
{
    m_model = model;
    setModel(m_model);

    // 各列宽度策略：标题和备注伸缩，URL 可调，文件夹/标签自适应内容
    horizontalHeader()->setStretchLastSection(false);
    const int colCount = model ? model->columnCount() : 0;
    for (int i = 0; i < colCount; i++) {
        if (i == 0 || i == colCount - 1)
            horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        else if (i == 1)
            horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
        else
            horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    // 设置默认宽度（Interactive 列）
    if (colCount > 1) setColumnWidth(1, 220);
}
