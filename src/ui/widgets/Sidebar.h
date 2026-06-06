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

signals:
    void allLinksRequested();
    void recentLinksRequested();
    void frequentLinksRequested();
    void folderSelected(int folderId);
    void tagSelected(int tagId);

private:
    void setupSmartList();
    void setupFolderTree();
    void setupTagList();

    QTreeWidget *m_smartList;
    QTreeWidget *m_folderTree;
    QListWidget *m_tagList;
    IFolderRepository *m_folderRepo = nullptr;
    ITagRepository    *m_tagRepo = nullptr;
};
