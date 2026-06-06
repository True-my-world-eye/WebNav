// LinkListView.h — 链接列表视图
// 以表格形式展示链接，列：favicon/标题/URL/文件夹/标签/日期

#pragma once
#include <QTableView>
#include <QStandardItemModel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDrag>
#include <QPoint>

class LinkListView : public QTableView
{
    Q_OBJECT

public:
    explicit LinkListView(QWidget *parent = nullptr);

    // 设置数据源模型
    void setLinkData(QStandardItemModel *model);

signals:
    void linkDoubleClicked(int linkId);
    // 拖拽完成时发出：请求将 linkId 移动到 targetRow 位置
    void linkMoveRequested(int linkId, int targetRow);

protected:
    void dropEvent(QDropEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;

private:
    int dragSourceRow() const { return m_dragSourceRow; }
    void setDragSourceRow(int row) { m_dragSourceRow = row; }

    QStandardItemModel *m_model = nullptr;
    int m_dragSourceRow = -1;
};
