// LinkListView.h — 链接列表视图
// 以表格形式展示链接，列：favicon/标题/URL/文件夹/标签/日期

#pragma once
#include <QTableView>
#include <QStandardItemModel>
#include <QDragEnterEvent>
#include <QDropEvent>

class LinkListView : public QTableView
{
    Q_OBJECT

public:
    explicit LinkListView(QWidget *parent = nullptr);

    // 设置数据源模型
    void setLinkData(QStandardItemModel *model);

signals:
    void linkDoubleClicked(int linkId);
    void linkDropped(int fromRow, int toRow);

protected:
    void dropEvent(QDropEvent *event) override;

private:
    QStandardItemModel *m_model = nullptr;
};

