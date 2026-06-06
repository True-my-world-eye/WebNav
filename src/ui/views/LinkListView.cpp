// LinkListView.cpp

#include "LinkListView.h"
#include <QHeaderView>
#include <QDrag>

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
    setDragDropOverwriteMode(false);

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

void LinkListView::startDrag(Qt::DropActions supportedActions)
{
    // 记录拖拽源行
    QModelIndex idx = currentIndex();
    m_dragSourceRow = idx.isValid() ? idx.row() : -1;
    QTableView::startDrag(supportedActions);
}

void LinkListView::dropEvent(QDropEvent *event)
{
    if (!m_model || m_dragSourceRow < 0) {
        QTableView::dropEvent(event);
        return;
    }

    int sourceRow = m_dragSourceRow;

    // 计算目标行：根据 drop indicator 位置
    QPoint pos = event->position().toPoint();
    QModelIndex destIdx = indexAt(pos);
    int targetRow = -1;

    if (destIdx.isValid()) {
        targetRow = destIdx.row();
        // 判断鼠标在目标行的上半还是下半
        QRect itemRect = visualRect(destIdx);
        int midY = itemRect.top() + itemRect.height() / 2;
        if (pos.y() > midY) {
            targetRow++; // 插入到该行下方
        }
    } else {
        // 拖到空白区域 = 放到末尾
        targetRow = m_model->rowCount();
    }

    // 阻止 Qt 默认的 InternalMove（它太不可靠）
    event->ignore();

    // 自己执行行移动：取出行，插入到目标位置
    if (targetRow < 0) return;

    // 收集该行的所有 QStandardItem
    int cols = m_model->columnCount();
    QVector<QStandardItem*> rowItems;
    rowItems.reserve(cols);
    for (int c = 0; c < cols; c++) {
        rowItems.append(m_model->takeItem(sourceRow, c));
    }
    m_model->removeRow(sourceRow);

    // 如果 targetRow 大于 sourceRow，因为已经删了一行，targetRow 需要 -1
    int insertRow = targetRow;
    if (targetRow > sourceRow) {
        insertRow = targetRow - 1;
    }
    // 边界处理
    if (insertRow < 0) insertRow = 0;
    if (insertRow > m_model->rowCount()) insertRow = m_model->rowCount();

    // 在目标位置插入
    for (int c = 0; c < cols; c++) {
        m_model->setItem(insertRow, c, rowItems[c]);
    }

    // 选中移动后的行
    QModelIndex newIdx = m_model->index(insertRow, 0);
    selectionModel()->setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);

    // 发出信号让 MainWindow 持久化
    int linkId = m_model->item(insertRow, 0)->data(Qt::UserRole).toInt();
    emit linkMoveRequested(linkId, insertRow);
}
