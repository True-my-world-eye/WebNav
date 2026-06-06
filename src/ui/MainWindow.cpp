// MainWindow.cpp

#include "MainWindow.h"
#include "LinkEditDialog.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "PlatformUtils.h"

#include <QToolBar>
#include <QPushButton>
#include <QStatusBar>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QShortcut>

MainWindow::MainWindow(ILinkRepository *linkRepo,
                        IFolderRepository *folderRepo,
                        ITagRepository *tagRepo,
                        QWidget *parent)
    : QMainWindow(parent)
    , m_linkRepo(linkRepo)
    , m_folderRepo(folderRepo)
    , m_tagRepo(tagRepo)
{
    setWindowTitle(QStringLiteral("WebNav \u2014 \u684c\u9762\u7f51\u9875\u94fe\u63a5\u7ba1\u7406\u5668"));
    resize(1100, 700);
    setMinimumSize(800, 500);

    setupToolBar();
    setupStatusBar();
    setupShortcuts();

    // ── 中央区域：侧边栏 + 视图栈 ──
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_sidebar = new Sidebar(this);
    splitter->addWidget(m_sidebar);

    // 视图栈（列表/卡片）
    m_viewStack = new QStackedWidget(this);
    m_listView = new LinkListView(this);
    m_cardView = new LinkCardView(this);
    m_viewStack->addWidget(m_listView);     // index 0: 列表
    m_viewStack->addWidget(m_cardView);     // index 1: 卡片
    splitter->addWidget(m_viewStack);

    splitter->setStretchFactor(0, 0);       // 侧边栏不伸缩
    splitter->setStretchFactor(1, 1);       // 视图区域伸缩
    splitter->setSizes({220, 880});

    setCentralWidget(splitter);

    // ── 信号连接 ──
    connect(m_listView, &LinkListView::linkDoubleClicked,
            this, &MainWindow::openLink);
    connect(m_cardView, &LinkCardView::linkDoubleClicked,
            this, &MainWindow::openLink);
    connect(m_sidebar, &Sidebar::allLinksRequested,
            this, &MainWindow::refreshLinks);
    connect(m_sidebar, &Sidebar::recentLinksRequested,
            this, &MainWindow::refreshLinks);
    connect(m_sidebar, &Sidebar::frequentLinksRequested,
            this, &MainWindow::refreshLinks);
    connect(m_sidebar, &Sidebar::folderSelected, this, [this](int) {
        refreshLinks();
    });
    connect(m_sidebar, &Sidebar::tagSelected, this, [this](int) {
        refreshLinks();
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar(QStringLiteral("\u5de5\u5177\u680f"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // ── 搜索栏 ──
    m_searchBar = new SearchBar(this);
    m_searchBar->setFixedWidth(300);
    connect(m_searchBar, &SearchBar::searchTriggered,
            this, &MainWindow::onSearch);
    toolbar->addWidget(m_searchBar);

    toolbar->addSeparator();

    // ── 新建链接按钮 ──
    auto *newAction = toolbar->addAction(QStringLiteral("+ \u65b0\u5efa"));
    newAction->setToolTip(QStringLiteral("\u65b0\u5efa\u94fe\u63a5 (Ctrl+N)"));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewLink);

    // ── 视图切换按钮 ──
    auto *viewAction = toolbar->addAction(QStringLiteral("\U0001F4CB \u5217\u8868"));
    viewAction->setToolTip(QStringLiteral("\u5207\u6362\u89c6\u56fe (Ctrl+1/2)"));
    connect(viewAction, &QAction::triggered, this, [this, viewAction]() {
        toggleView();
        bool isList = !m_isCardView;
        viewAction->setText(isList ? QStringLiteral("\U0001F4CB \u5217\u8868")
                                   : QStringLiteral("\U0001F5BC \u5361\u7247"));
    });

    toolbar->addSeparator();

    // ── 设置 ──
    auto *settingsAction = toolbar->addAction(QStringLiteral("\u2699 \u8bbe\u7f6e"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(QStringLiteral("\u51c6\u5907\u5c31\u7eea"));
}

void MainWindow::setupShortcuts()
{
    // Ctrl+N 新建链接
    auto *newShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(newShortcut, &QShortcut::activated, this, &MainWindow::onNewLink);

    // Ctrl+F 搜索
    auto *searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, this, [this]() {
        m_searchBar->setFocus();
        m_searchBar->selectAll();
    });

    // Ctrl+1 列表视图
    auto *listShortcut = new QShortcut(QKeySequence("Ctrl+1"), this);
    connect(listShortcut, &QShortcut::activated, this, [this]() {
        if (m_isCardView) toggleView();
    });

    // Ctrl+2 卡片视图
    auto *cardShortcut = new QShortcut(QKeySequence("Ctrl+2"), this);
    connect(cardShortcut, &QShortcut::activated, this, [this]() {
        if (!m_isCardView) toggleView();
    });

    // Delete 删除
    auto *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(deleteShortcut, &QShortcut::activated, this, [this]() {
        // TODO: 删除选中链接
    });
}

void MainWindow::onNewLink()
{
    LinkEditDialog dialog(this);

    // 传入可用的文件夹列表
    QVector<Folder> folders = m_folderRepo->getAll();
    dialog.setFolders(folders);

    QVector<Tag> tags = m_tagRepo->getAll();
    dialog.setTags(tags);

    if (dialog.exec() == QDialog::Accepted)
    {
        Link link = dialog.link();
        int id = m_linkRepo->insert(link);
        if (id > 0)
        {
            statusBar()->showMessage(QStringLiteral("\u94fe\u63a5\u5df2\u4fdd\u5b58"), 3000);
            refreshLinks();
        }
        else
        {
            QMessageBox::warning(this,
                QStringLiteral("\u4fdd\u5b58\u5931\u8d25"),
                QStringLiteral("\u65e0\u6cd5\u4fdd\u5b58\u94fe\u63a5\uff0c\u8bf7\u68c0\u67e5\u6570\u636e\u5e93\u72b6\u6001\u3002"));
        }
    }
}

void MainWindow::toggleView()
{
    m_isCardView = !m_isCardView;
    m_viewStack->setCurrentIndex(m_isCardView ? 1 : 0);
    statusBar()->showMessage(
        m_isCardView ? QStringLiteral("\u5361\u7247\u89c6\u56fe")
                     : QStringLiteral("\u5217\u8868\u89c6\u56fe"),
        2000);
}

void MainWindow::onSearch(const QString &keyword)
{
    if (keyword.isEmpty())
    {
        refreshLinks();
        return;
    }

    QVector<Link> results = m_linkRepo->search(keyword);
    statusBar()->showMessage(
        QStringLiteral("\u641c\u7d22\u201c%1\u201d \u2014 %2 \u6761\u7ed3\u679c")
            .arg(keyword).arg(results.size()));
}

void MainWindow::refreshLinks()
{
    QVector<Link> links = m_linkRepo->getAll();
    statusBar()->showMessage(
        QStringLiteral("\u5171 %1 \u6761\u94fe\u63a5").arg(links.size()));
}

void MainWindow::openLink(int linkId)
{
    auto link = m_linkRepo->getById(linkId);
    if (link.has_value())
    {
        // 更新访问计数
        Link updatedLink = link.value();
        updatedLink.visitCount++;
        updatedLink.lastVisitedAt = QDateTime::currentDateTime();
        m_linkRepo->update(updatedLink);

        // 在浏览器中打开
        PlatformUtils::openInBrowser(link->url);
    }
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::openAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

