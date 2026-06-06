// MainWindow.h — 应用主窗口
// 集成工具栏、侧边栏、双视图、状态栏，协调所有 UI 交互

#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
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
    explicit MainWindow(ILinkRepository *linkRepo,
                        IFolderRepository *folderRepo,
                        ITagRepository *tagRepo,
                        QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:
    // 新建链接
    void onNewLink();

    // 切换视图
    void toggleView();

    // 搜索
    void onSearch(const QString &keyword);

    // 刷新链接列表
    void refreshLinks();

    // 打开链接
    void openLink(int linkId);

    // 打开设置
    void openSettings();

    // 打开关于
    void openAbout();

private:
    void setupToolBar();
    void setupStatusBar();
    void setupShortcuts();

    // 仓库
    ILinkRepository   *m_linkRepo;
    IFolderRepository *m_folderRepo;
    ITagRepository    *m_tagRepo;

    // UI 组件
    Sidebar       *m_sidebar;
    SearchBar     *m_searchBar;
    QStackedWidget *m_viewStack;     // 列表/卡片视图切换
    LinkListView  *m_listView;
    LinkCardView  *m_cardView;

    // 状态
    bool m_isCardView = false;      // 当前是否为卡片视图
};

