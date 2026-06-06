// LinkListView.cpp

#include "LinkListView.h"
#include <QHeaderView>

LinkListView::LinkListView(QWidget *parent)
    : QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAlternatingRowColors(true);
    setSortingEnabled(false);
    verticalHeader()->hide();
    setShowGrid(false);
    horizontalHeader()->setMinimumSectionSize(60);

    // 启用拖拽
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(true);   // true = insert 行，false = 尝试覆盖（Qt bug 多）

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

    // 列 0-3 可手动调整，最后一列(备注)自动伸缩填满剩余空间
    horizontalHeader()->setStretchLastSection(false);
    const int colCount = model ? model->columnCount() : 0;
    for (int i = 0; i < colCount; i++) {
        if (i == colCount - 1)
            horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        else
            horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
    }
}

void LinkListView::dropEvent(QDropEvent *event)
{
    if (!m_model) return;

    // 记录拖拽前的行数据（linkId 列表）
    QVector<int> oldIds;
    for (int r = 0; r < m_model->rowCount(); r++) {
        int id = m_model->item(r, 0)->data(Qt::UserRole).toInt();
        oldIds.append(id);
    }

    // 执行默认 drop
    QTableView::dropEvent(event);

    if (!event->isAccepted()) return;

    // 拖拽后 model 的行被 Qt 重新排列了
    // 读取新顺序的 linkId
    QVector<int> newIds;
    for (int r = 0; r < m_model->rowCount(); r++) {
        int id = m_model->item(r, 0)->data(Qt::UserRole).toInt();
        newIds.append(id);
    }

    // 找出哪些行的位置变了
    int srcRow = -1, dstRow = -1;
    for (int i = 0; i < newIds.size(); i++) {
        if (i < oldIds.size() && newIds[i] != oldIds[i]) {
            // 这个位置变了：在 oldIds 中找到 newIds[i] 的原始位置
            int oldPos = oldIds.indexOf(newIds[i]);
            if (oldPos >= 0 && oldPos != i) {
                srcRow = oldPos;
                dstRow = i;
                break;
            }
        }
    }

    emit linkDropped(srcRow, dstRow);
}
