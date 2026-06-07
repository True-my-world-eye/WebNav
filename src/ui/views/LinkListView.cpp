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
    event->ignore(); // 阻止 Qt 默认 InternalMove（不可靠）

    // ── 根据鼠标 Y 坐标判断插入位置 ──
    QPoint pos = event->position().toPoint();
    int totalRows = m_model->rowCount();
    int targetRow = totalRows; // 默认末尾

    for (int r = 0; r < totalRows; r++) {
        QModelIndex idx = m_model->index(r, 0);
        QRect rect = visualRect(idx);
        if (rect.isValid() && pos.y() >= rect.top() && pos.y() <= rect.bottom()) {
            // 鼠标在这个行的垂直范围内
            int midY = rect.top() + rect.height() / 2;
            targetRow = (pos.y() < midY) ? r : r + 1;
            break;
        }
    }

    // 不要移动到自己原来的位置
    if (targetRow == sourceRow || targetRow == sourceRow + 1) {
        return;
    }

    // ── 安全行移动：读数据 → 删行 → 插行 → 写数据 ──
    int cols = m_model->columnCount();

    // 1. 读取源行全部 cell 数据
    QStringList texts;
    QStringList tips;
    QVector<QVariant> roleData;
    for (int c = 0; c < cols; c++) {
        QStandardItem *item = m_model->item(sourceRow, c);
        if (item) {
            texts << item->text();
            tips << item->toolTip();
            roleData.append(item->data(Qt::UserRole));
        } else {
            texts << QString();
            tips << QString();
            roleData.append(QVariant());
        }
    }

    // 2. 删除源行
    m_model->removeRow(sourceRow);

    // 3. 修正目标行索引（已删一行）
    if (targetRow > sourceRow)
        targetRow--;
    if (targetRow < 0) targetRow = 0;
    if (targetRow > m_model->rowCount()) targetRow = m_model->rowCount();

    // 4. 插入新行
    m_model->insertRow(targetRow);

    // 5. 填入数据
    for (int c = 0; c < cols; c++) {
        if (c < texts.size() && !texts[c].isEmpty()) {
            auto *newItem = new QStandardItem(texts[c]);
            newItem->setToolTip(tips[c]);
            newItem->setData(roleData[c], Qt::UserRole);
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
            m_model->setItem(targetRow, c, newItem);
        }
    }

    // 选中移动后的行
    QModelIndex newIdx = m_model->index(targetRow, 0);
    selectionModel()->setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);

    emit linkMoveRequested(0, targetRow);
}
