#pragma once
#include <QTreeWidget>

class IFolderRepository;

class FolderTreeView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FolderTreeView(QWidget *parent = nullptr);
    void setFolderRepository(IFolderRepository *repo) { m_repo = repo; }
    void refresh();
signals:
    void folderSelected(int folderId);
    void newFolderRequested(int parentId);
    void renameFolderRequested(int folderId);
    void deleteFolderRequested(int folderId);
private:
    void addFolderNode(int parentFolderId, QTreeWidgetItem *parentItem);
    IFolderRepository *m_repo = nullptr;
};
