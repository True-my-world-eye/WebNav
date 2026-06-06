#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QLabel>

class IFolderRepository;
class ITagRepository;

class Sidebar : public QWidget
{
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);
    void setRepositories(IFolderRepository *folderRepo, ITagRepository *tagRepo);
    void refresh();
    // 获取当前选中的筛选 ID（-1 表示无筛选）
    int currentFolderId() const { return m_selectedFolderId; }
    int currentTagId() const { return m_selectedTagId; }

signals:
    void allLinksRequested();
    void folderStructureChanged();
    void folderSelected(int folderId);      // -1 表示取消选中
    void tagSelected(int tagId);            // -1 表示取消选中
    void folderNewRequested(int parentId);
    void folderRenameRequested(int folderId);
    void folderDeleteRequested(int folderId);
    void tagDeleteRequested(int tagId);

private:
    void setupFolderTree();
    void setupTagList();

    QTreeWidget *m_folderTree;
    QListWidget *m_tagList;
    IFolderRepository *m_folderRepo = nullptr;
    ITagRepository    *m_tagRepo = nullptr;
    int m_selectedFolderId = -1;
    int m_selectedTagId = -1;
};
