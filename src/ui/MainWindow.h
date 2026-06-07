// MainWindow.h — 主窗口框架

#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QAction>
#include <QTimer>
#include <QClipboard>
#include "Sidebar.h"
#include "services/FaviconService.h"
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
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNewLink();
    void toggleView();
    void onSearch(const QString &keyword);
    void refreshLinks();
    void openSettings();
    void openHelp();
    void onImportBookmarks();
    void onExportBookmarks();
    void onNewFolder(int parentId);
    void onRenameFolder(int folderId);
    void onDeleteFolder(int folderId);
    void onDeleteTag(int tagId);
    void onEditLink();
    void onOpenLink();
    void onDeleteLink();
    void showContextMenu(const QPoint &pos);
    void onDoubleClicked(int linkId);
    void updateStatusBar();

private:
    void setupToolBar();
    void setupStatusBar();
    void setupShortcuts();
    void buildLinkModel(const QVector<Link> &links);
    void applyFilters();
    void saveLinkOrder();
    void moveSelectedLink(int direction);
    void batchOpen(const QVector<int> &ids);
    void batchTag(const QVector<int> &ids);
    void batchMoveFolder(const QVector<int> &ids);
    int selectedLinkId() const;
    QVector<int> selectedLinkIds() const;

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
    QAction *m_editAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_deleteAction = nullptr;
    bool m_isCardView = false;
    bool m_isRebuildingModel = false;

    // 筛选状态
    int m_filterFolderId = -1;
    int m_filterTagId = -1;
    QString m_filterKeyword;

    QTimer *m_dragRefreshTimer = nullptr;
    FaviconService *m_faviconService = nullptr;
};
