#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QAction>
#include "Sidebar.h"
#include "SearchBar.h"
#include "LinkListView.h"
#include "LinkCardView.h"
#include "database/interfaces/ILinkRepository.h"
#include "database/interfaces/IFolderRepository.h"
#include "database/interfaces/ITagRepository.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(ILinkRepository *linkRepo, IFolderRepository *folderRepo,
               ITagRepository *tagRepo, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewLink();
    void toggleView();
    void onSearch(const QString &keyword);
    void refreshLinks();
    void openLink(int linkId);
    void openSettings();
    void openAbout();
    void onNewFolder(int parentId);
    void onRenameFolder(int folderId);
    void onDeleteFolder(int folderId);

private:
    void setupToolBar();
    void setupStatusBar();
    void setupShortcuts();
    void buildLinkModel(const QVector<Link> &links);

    ILinkRepository   *m_linkRepo;
    IFolderRepository *m_folderRepo;
    ITagRepository    *m_tagRepo;

    Sidebar       *m_sidebar;
    SearchBar     *m_searchBar;
    QStackedWidget *m_viewStack;
    LinkListView  *m_listView;
    LinkCardView  *m_cardView;
    QStandardItemModel *m_linkModel = nullptr;
    QAction *m_viewAction = nullptr;
    bool m_isCardView = false;
};
