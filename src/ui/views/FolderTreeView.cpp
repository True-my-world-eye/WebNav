// FolderTreeView.cpp

#include "FolderTreeView.h"
#include <QMenu>
#include <QHeaderView>

FolderTreeView::FolderTreeView(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setAnimated(true);
    setIndentation(16);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // 根级"全部文件夹"占位
    auto *rootItem = new QTreeWidgetItem(this);
    rootItem->setText(0, QStringLiteral("\U0001F4C1 \u5168\u90e8\u6587\u4ef6\u5939"));
    rootItem->setData(0, Qt::UserRole, -1);

    connect(this, &QTreeWidget::itemClicked,
            this, &FolderTreeView::onItemClicked);
    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &FolderTreeView::onCustomContextMenu);
}

void FolderTreeView::refresh()
{
    // 清除现有节点
    clear();

    // 创建根节点
    auto *rootItem = new QTreeWidgetItem(this);
    rootItem->setText(0, QStringLiteral("\U0001F4C1 \u5168\u90e8\u6587\u4ef6\u5939"));
    rootItem->setData(0, Qt::UserRole, -1);

    // TODO: 从 SqliteFolderRepository 读取文件夹并递归添加到树
    // addFolderNode(-1, rootItem);

    expandAll();
}

void FolderTreeView::addFolderNode(int parentFolderId, QTreeWidgetItem *parentItem)
{
    Q_UNUSED(parentFolderId)
    Q_UNUSED(parentItem)
    // TODO: Phase 1 实现
    // 从 SqliteFolderRepository 的 getByParent(parentFolderId) 获取子文件夹
    // 为每个子文件夹创建 QTreeWidgetItem
    // 递归调用 addFolderNode
}

void FolderTreeView::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    int folderId = item->data(0, Qt::UserRole).toInt();
    emit folderSelected(folderId);
}

void FolderTreeView::onCustomContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = itemAt(pos);
    if (!item) return;

    int folderId = item->data(0, Qt::UserRole).toInt();
    emit folderContextMenu(folderId, mapToGlobal(pos));
}
