// FolderTreeView.h — 文件夹树视图
// 显示文件夹层级结构，支持拖拽排序和右键菜单

#pragma once
#include <QTreeWidget>

class FolderTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit FolderTreeView(QWidget *parent = nullptr);

    // 刷新文件夹树
    void refresh();

signals:
    void folderSelected(int folderId);
    void folderContextMenu(int folderId, const QPoint &pos);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onCustomContextMenu(const QPoint &pos);

private:
    // 递归添加文件夹节点
    void addFolderNode(int parentFolderId, QTreeWidgetItem *parentItem);
};

