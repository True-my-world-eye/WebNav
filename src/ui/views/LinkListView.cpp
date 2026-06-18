// LinkListView.cpp — 列表视图实现
// 以表格形式展示链接列表，支持：
// - 只读模式（NoEditTriggers）
// - 多选支持（ExtendedSelection）
// - 拖拽排序（InternalMove）
// - 双击编辑
// - 右键菜单

#include "LinkListView.h"
#include <QHeaderView>
#include <QDrag>

// ── 构造函数 ──────────────────────────────────────────────────
// 初始化列表视图，设置选择模式和拖拽支持
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

// ── 设置链接数据 ──────────────────────────────────────────────
// 绑定数据模型到视图，设置列宽
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

// ── 拖拽实现 ──────────────────────────────────────────────────

// 开始拖拽
// 记录源行索引，用于后续的行移动操作
void LinkListView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndex idx = currentIndex();
    m_dragSourceRow = idx.isValid() ? idx.row() : -1;
    QTableView::startDrag(supportedActions);
}

// 拖拽放下事件
// 实现行移动逻辑，替代 Qt 内置的 InternalMove（不可靠）
// 流程：
// 1. 根据鼠标 Y 坐标计算目标行
// 2. 读取源行数据
// 3. 删除源行
// 4. 插入到目标位置
// 5. 发射信号通知持久化
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
