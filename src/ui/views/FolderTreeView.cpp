#include "FolderTreeView.h"
#include "database/interfaces/IFolderRepository.h"
#include "models/Folder.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

FolderTreeView::FolderTreeView(QWidget *parent) : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setAnimated(true);
    setIndentation(16);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        emit folderSelected(item->data(0, Qt::UserRole).toInt());
    });
    connect(this, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        auto *item = itemAt(pos);
        int fid = item ? item->data(0, Qt::UserRole).toInt() : -1;
        QMenu menu;
        auto *newAct = menu.addAction(QStringLiteral("\u65b0\u5efa\u5b50\u6587\u4ef6\u5939"));
        if (fid > 0) {
            menu.addSeparator();
            auto *renAct = menu.addAction(QStringLiteral("\u91cd\u547d\u540d"));
            auto *delAct = menu.addAction(QStringLiteral("\u5220\u9664"));
            connect(renAct, &QAction::triggered, this, [this, fid]() {
                emit renameFolderRequested(fid);
            });
            connect(delAct, &QAction::triggered, this, [this, fid]() {
                emit deleteFolderRequested(fid);
            });
        }
        connect(newAct, &QAction::triggered, this, [this, fid]() {
            emit newFolderRequested(fid);
        });
        menu.exec(mapToGlobal(pos));
    });
}

void FolderTreeView::refresh()
{
    clear();
    if (!m_repo) return;
    addFolderNode(-1, nullptr);
    expandAll();
}

void FolderTreeView::addFolderNode(int parentFolderId, QTreeWidgetItem *parentItem)
{
    auto folders = m_repo->getByParent(parentFolderId);
    for (const auto &f : folders)
    {
        auto *item = parentItem
            ? new QTreeWidgetItem(parentItem)
            : new QTreeWidgetItem(this);
        item->setText(0, f.name);
        item->setData(0, Qt::UserRole, f.id);
        addFolderNode(f.id, item);
    }
}
