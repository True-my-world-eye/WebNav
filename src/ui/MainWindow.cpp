#include "MainWindow.h"
#include "LinkEditDialog.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "PlatformUtils.h"
#include <QToolBar>
#include <QPushButton>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QShortcut>
#include <QInputDialog>
#include "models/LinkField.h"
#include "models/Tag.h"

MainWindow::MainWindow(ILinkRepository *linkRepo, IFolderRepository *folderRepo,
                       ITagRepository *tagRepo, QWidget *parent)
    : QMainWindow(parent), m_linkRepo(linkRepo), m_folderRepo(folderRepo), m_tagRepo(tagRepo)
{
    setWindowTitle(QStringLiteral("WebNav \u2014 \u684c\u9762\u7f51\u9875\u94fe\u63a5\u7ba1\u7406\u5668"));
    resize(1100, 700);
    setMinimumSize(800, 500);
    setupToolBar();
    setupStatusBar();
    setupShortcuts();

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    m_sidebar = new Sidebar(this);
    m_sidebar->setRepositories(m_folderRepo, m_tagRepo);
    splitter->addWidget(m_sidebar);

    m_viewStack = new QStackedWidget(this);
    m_listView = new LinkListView(this);
    m_cardView = new LinkCardView(this);
    m_viewStack->addWidget(m_listView);
    m_viewStack->addWidget(m_cardView);
    splitter->addWidget(m_viewStack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({220, 880});
    setCentralWidget(splitter);

    // ── 信号连接 ──
    connect(m_listView, &LinkListView::linkDoubleClicked, this, &MainWindow::openLink);
    connect(m_cardView, &LinkCardView::linkDoubleClicked, this, &MainWindow::openLink);
    connect(m_sidebar, &Sidebar::allLinksRequested, this, &MainWindow::refreshLinks);
    connect(m_sidebar, &Sidebar::recentLinksRequested, this, [this]() {
        buildLinkModel(m_linkRepo->getRecent(50));
    });
    connect(m_sidebar, &Sidebar::frequentLinksRequested, this, [this]() {
        buildLinkModel(m_linkRepo->getMostVisited(50));
    });
    connect(m_sidebar, &Sidebar::folderSelected, this, [this](int fid) {
        buildLinkModel(m_linkRepo->getByFolder(fid));
    });
    connect(m_sidebar, &Sidebar::tagSelected, this, [this](int) {
        refreshLinks();
    });

    refreshLinks();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupToolBar()
{
    auto *tb = addToolBar(QStringLiteral("\u5de5\u5177\u680f"));
    tb->setMovable(false);
    tb->setIconSize(QSize(16, 16));
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_searchBar = new SearchBar(this);
    m_searchBar->setFixedWidth(300);
    connect(m_searchBar, &SearchBar::searchTriggered, this, &MainWindow::onSearch);
    tb->addWidget(m_searchBar);
    tb->addSeparator();

    auto *newAct = tb->addAction(QStringLiteral("+ \u65b0\u5efa"));
    newAct->setToolTip(QStringLiteral("\u65b0\u5efa\u94fe\u63a5 (Ctrl+N)"));
    connect(newAct, &QAction::triggered, this, &MainWindow::onNewLink);

    m_viewAction = tb->addAction(QStringLiteral("\U0001F4CB \u5217\u8868"));
    m_viewAction->setToolTip(QStringLiteral("\u5207\u6362\u89c6\u56fe (Ctrl+1/2)"));
    connect(m_viewAction, &QAction::triggered, this, &MainWindow::toggleView);
    tb->addSeparator();

    auto *settingsAct = tb->addAction(QStringLiteral("\u2699 \u8bbe\u7f6e"));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::openSettings);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(QStringLiteral("\u51c6\u5907\u5c31\u7eea"));
}

void MainWindow::setupShortcuts()
{
    auto *newSC = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(newSC, &QShortcut::activated, this, &MainWindow::onNewLink);

    auto *searchSC = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchSC, &QShortcut::activated, this, [this]() {
        m_searchBar->setFocus(); m_searchBar->selectAll();
    });

    connect(new QShortcut(QKeySequence("Ctrl+1"), this), &QShortcut::activated, this, [this]() {
        if (m_isCardView) toggleView();
    });
    connect(new QShortcut(QKeySequence("Ctrl+2"), this), &QShortcut::activated, this, [this]() {
        if (!m_isCardView) toggleView();
    });
}

void MainWindow::buildLinkModel(const QVector<Link> &links)
{
    delete m_linkModel;
    m_linkModel = new QStandardItemModel(links.size(), 5, this);
    m_linkModel->setHorizontalHeaderLabels({
        QStringLiteral("\u6807\u9898"), QStringLiteral("URL"),
        QStringLiteral("\u6587\u4ef6\u5939"), QStringLiteral("\u6807\u7b7e"),
        QStringLiteral("\u65f6\u95f4")});

    QMap<int, QString> folderNames;
    if (m_folderRepo) {
        for (const auto &f : m_folderRepo->getAll())
            folderNames[f.id] = f.name;
    }

    for (int i = 0; i < links.size(); i++)
    {
        const auto &link = links[i];
        auto *titleItem = new QStandardItem(link.title.isEmpty() ? link.url : link.title);
        titleItem->setData(link.id, Qt::UserRole);
        titleItem->setToolTip(link.url);
        m_linkModel->setItem(i, 0, titleItem);
        m_linkModel->setItem(i, 1, new QStandardItem(link.url));

        QString folderName = folderNames.value(link.folderId);
        m_linkModel->setItem(i, 2, new QStandardItem(folderName));

        QStringList tagNames;
        if (m_tagRepo) {
            for (const auto &t : m_tagRepo->getTagsForLink(link.id))
                tagNames << t.name;
        }
        m_linkModel->setItem(i, 3, new QStandardItem(tagNames.join(", ")));

        QString timeStr = link.createdAt.isValid()
            ? link.createdAt.toString("yyyy-MM-dd") : QString();
        m_linkModel->setItem(i, 4, new QStandardItem(timeStr));
    }

    m_listView->setLinkData(m_linkModel);
    m_cardView->setLinkData(m_linkModel);

    statusBar()->showMessage(
        QStringLiteral("\u5171 %1 \u6761\u94fe\u63a5").arg(links.size()));
}

void MainWindow::refreshLinks()
{
    buildLinkModel(m_linkRepo->getAll());
}

void MainWindow::onSearch(const QString &keyword)
{
    if (keyword.isEmpty()) { refreshLinks(); return; }
    buildLinkModel(m_linkRepo->search(keyword));
}

void MainWindow::onNewLink()
{
    LinkEditDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u65b0\u5efa\u94fe\u63a5"));
    dialog.setFolders(m_folderRepo ? m_folderRepo->getAll() : QVector<Folder>());
    dialog.setTags(m_tagRepo ? m_tagRepo->getAll() : QVector<Tag>());

    connect(&dialog, &LinkEditDialog::createNewTag, this, [this](const QString &name) {
        Tag t; t.name = name;
        int tid = m_tagRepo->insert(t);
        if (tid > 0) {
            QVector<Tag> tags = m_tagRepo->getAll();
            (void)tid;
        }
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        Link link = dialog.link();
        int id = m_linkRepo->insert(link);
        if (id > 0)
        {
            // 保存标签关联
            for (int tagId : dialog.selectedTagIds())
                m_tagRepo->addTagToLink(id, tagId);

            // 保存附加字段
            auto fields = dialog.linkFields();
            for (auto &f : fields)
                f.linkId = id;
            m_linkRepo->saveLinkFields(id, fields);

            statusBar()->showMessage(QStringLiteral("\u94fe\u63a5\u5df2\u4fdd\u5b58"), 3000);
            refreshLinks();
            m_sidebar->refresh();
        }
    }
}

void MainWindow::toggleView()
{
    m_isCardView = !m_isCardView;
    m_viewStack->setCurrentIndex(m_isCardView ? 1 : 0);
    if (m_viewAction)
        m_viewAction->setText(m_isCardView
            ? QStringLiteral("\U0001F5BC \u5361\u7247")
            : QStringLiteral("\U0001F4CB \u5217\u8868"));
}

void MainWindow::openLink(int linkId)
{
    auto opt = m_linkRepo->getById(linkId);
    if (opt.has_value())
    {
        Link l = opt.value();
        l.visitCount++;
        l.lastVisitedAt = QDateTime::currentDateTime();
        m_linkRepo->update(l);
        PlatformUtils::openInBrowser(l.url);
    }
}

void MainWindow::onNewFolder(int parentId)
{
    bool ok;
    QString name = QInputDialog::getText(this,
        QStringLiteral("\u65b0\u5efa\u6587\u4ef6\u5939"),
        QStringLiteral("\u6587\u4ef6\u5939\u540d\u79f0:"), QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        Folder f; f.name = name.trimmed(); f.parentId = parentId;
        m_folderRepo->insert(f);
        m_sidebar->refresh();
    }
}

void MainWindow::onRenameFolder(int folderId)
{
    auto opt = m_folderRepo->getById(folderId);
    if (!opt.has_value()) return;
    bool ok;
    QString newName = QInputDialog::getText(this,
        QStringLiteral("\u91cd\u547d\u540d\u6587\u4ef6\u5939"),
        QStringLiteral("\u65b0\u540d\u79f0:"), QLineEdit::Normal, opt->name, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        Folder f = opt.value(); f.name = newName.trimmed();
        m_folderRepo->update(f);
        m_sidebar->refresh();
    }
}

void MainWindow::onDeleteFolder(int folderId)
{
    auto ret = QMessageBox::question(this,
        QStringLiteral("\u5220\u9664\u6587\u4ef6\u5939"),
        QStringLiteral("\u786e\u5b9a\u8981\u5220\u9664\u6b64\u6587\u4ef6\u5939\u53ca\u5176\u6240\u6709\u5b50\u6587\u4ef6\u5939\u5417\uff1f"),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_folderRepo->remove(folderId);
        m_sidebar->refresh();
        refreshLinks();
    }
}

void MainWindow::openSettings()
{
    SettingsDialog d(this); d.exec();
}

void MainWindow::openAbout()
{
    AboutDialog d(this); d.exec();
}
