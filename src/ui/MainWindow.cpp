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
    setWindowTitle(QStringLiteral("WebNav"));
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

    // Both views: disable direct editing, enable context menu
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_cardView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cardView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Signals
    connect(m_listView, &LinkListView::linkDoubleClicked, this, &MainWindow::onDoubleClicked);
    connect(m_cardView, &LinkCardView::linkDoubleClicked, this, &MainWindow::onDoubleClicked);
    connect(m_listView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(m_cardView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
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

    m_viewAction = tb->addAction(QStringLiteral("\u5217\u8868"));
    m_viewAction->setToolTip(QStringLiteral("\u5207\u6362\u89c6\u56fe (Ctrl+1/2)"));
    connect(m_viewAction, &QAction::triggered, this, &MainWindow::toggleView);
    tb->addSeparator();

    m_editAction = tb->addAction(QStringLiteral("\u270f \u7f16\u8f91"));
    m_editAction->setToolTip(QStringLiteral("\u7f16\u8f91\u9009\u4e2d\u7684\u94fe\u63a5"));
    connect(m_editAction, &QAction::triggered, this, &MainWindow::onEditLink);

    m_openAction = tb->addAction(QStringLiteral("\U0001F310 \u6253\u5f00"));
    m_openAction->setToolTip(QStringLiteral("\u5728\u6d4f\u89c8\u5668\u4e2d\u6253\u5f00\u9009\u4e2d\u94fe\u63a5"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenLink);

    m_deleteAction = tb->addAction(QStringLiteral("\U0001F5D1 \u5220\u9664"));
    m_deleteAction->setToolTip(QStringLiteral("\u5220\u9664\u9009\u4e2d\u7684\u94fe\u63a5"));
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteLink);
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

    // Delete shortcut
    auto *delSC = new QShortcut(QKeySequence("Delete"), this);
    connect(delSC, &QShortcut::activated, this, &MainWindow::onDeleteLink);
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
        titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsEditable);
        m_linkModel->setItem(i, 0, titleItem);

        auto *urlItem = new QStandardItem(link.url);
        urlItem->setFlags(urlItem->flags() & ~Qt::ItemIsEditable);
        m_linkModel->setItem(i, 1, urlItem);

        QString folderName = folderNames.value(link.folderId);
        auto *folderItem = new QStandardItem(folderName);
        folderItem->setFlags(folderItem->flags() & ~Qt::ItemIsEditable);
        m_linkModel->setItem(i, 2, folderItem);

        QStringList tagNames;
        if (m_tagRepo) {
            for (const auto &t : m_tagRepo->getTagsForLink(link.id))
                tagNames << t.name;
        }
        auto *tagItem = new QStandardItem(tagNames.join(", "));
        tagItem->setFlags(tagItem->flags() & ~Qt::ItemIsEditable);
        m_linkModel->setItem(i, 3, tagItem);

        QString timeStr = link.createdAt.isValid()
            ? link.createdAt.toString("yyyy-MM-dd") : QString();
        auto *timeItem = new QStandardItem(timeStr);
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        m_linkModel->setItem(i, 4, timeItem);
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
        m_tagRepo->insert(t);
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        Link link = dialog.link();
        int id = m_linkRepo->insert(link);
        if (id > 0)
        {
            for (int tagId : dialog.selectedTagIds())
                m_tagRepo->addTagToLink(id, tagId);

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
            : QStringLiteral("\u5217\u8868"));
}

int MainWindow::selectedLinkId() const
{
    int viewIdx = m_viewStack->currentIndex();
    if (viewIdx == 0) {
        auto sel = m_listView->selectionModel();
        if (sel && sel->hasSelection())
            return sel->currentIndex().data(Qt::UserRole).toInt();
    } else {
        auto sel = m_cardView->selectionModel();
        if (sel && sel->hasSelection())
            return sel->currentIndex().data(Qt::UserRole).toInt();
    }
    return -1;
}

QVector<int> MainWindow::selectedLinkIds() const
{
    QVector<int> ids;
    int viewIdx = m_viewStack->currentIndex();
    QItemSelectionModel *sel = (viewIdx == 0)
        ? m_listView->selectionModel()
        : m_cardView->selectionModel();
    if (sel && sel->hasSelection()) {
        for (const QModelIndex &idx : sel->selectedRows())
            ids.append(idx.data(Qt::UserRole).toInt());
    }
    return ids;
}

void MainWindow::onDoubleClicked(int linkId)
{
    // Double-click opens edit dialog
    auto opt = m_linkRepo->getById(linkId);
    if (!opt.has_value()) return;
    Link link = opt.value();

    LinkEditDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u7f16\u8f91\u94fe\u63a5"));
    dialog.setLink(link);
    dialog.setFolders(m_folderRepo->getAll());
    dialog.setTags(m_tagRepo->getAll());
    dialog.fieldEditor()->setFields(m_linkRepo->getLinkFields(linkId));

    // Pre-select tags
    if (m_tagRepo) {
        QVector<int> tagIds;
        for (const auto &t : m_tagRepo->getTagsForLink(linkId))
            tagIds.append(t.id);
        dialog.tagSelector()->setSelectedTagIds(tagIds);
    }

    connect(&dialog, &LinkEditDialog::createNewTag, this, [this](const QString &name) {
        Tag t; t.name = name;
        m_tagRepo->insert(t);
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        Link updated = dialog.link();
        updated.id = linkId;
        updated.createdAt = link.createdAt;
        m_linkRepo->update(updated);

        // Update tags
        if (m_tagRepo) {
            for (const auto &t : m_tagRepo->getTagsForLink(linkId))
                m_tagRepo->removeTagFromLink(linkId, t.id);
            for (int tagId : dialog.selectedTagIds())
                m_tagRepo->addTagToLink(linkId, tagId);
        }

        // Update fields
        auto fields = dialog.linkFields();
        for (auto &f : fields) f.linkId = linkId;
        m_linkRepo->saveLinkFields(linkId, fields);

        statusBar()->showMessage(QStringLiteral("\u94fe\u63a5\u5df2\u66f4\u65b0"), 3000);
        refreshLinks();
    }
}

void MainWindow::onEditLink()
{
    int linkId = selectedLinkId();
    if (linkId <= 0) {
        statusBar()->showMessage(QStringLiteral("\u8bf7\u5148\u9009\u62e9\u4e00\u6761\u94fe\u63a5"), 3000);
        return;
    }
    onDoubleClicked(linkId);
}

void MainWindow::onOpenLink()
{
    int linkId = selectedLinkId();
    if (linkId <= 0) {
        statusBar()->showMessage(QStringLiteral("\u8bf7\u5148\u9009\u62e9\u4e00\u6761\u94fe\u63a5"), 3000);
        return;
    }
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

void MainWindow::onDeleteLink()
{
    QVector<int> ids = selectedLinkIds();
    if (ids.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("\u8bf7\u5148\u9009\u62e9\u8981\u5220\u9664\u7684\u94fe\u63a5"), 3000);
        return;
    }

    QString msg = (ids.size() == 1)
        ? QStringLiteral("\u786e\u5b9a\u8981\u5220\u9664\u8fd9\u6761\u94fe\u63a5\u5417\uff1f")
        : QStringLiteral("\u786e\u5b9a\u8981\u5220\u9664\u8fd9 %1 \u6761\u94fe\u63a5\u5417\uff1f").arg(ids.size());

    auto ret = QMessageBox::question(this,
        QStringLiteral("\u5220\u9664\u94fe\u63a5"), msg,
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_linkRepo->removeMultiple(ids);
        statusBar()->showMessage(
            QStringLiteral("\u5df2\u5220\u9664 %1 \u6761\u94fe\u63a5").arg(ids.size()), 3000);
        refreshLinks();
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index;
    int viewIdx = m_viewStack->currentIndex();
    if (viewIdx == 0)
        index = m_listView->indexAt(pos);
    else
        index = m_cardView->indexAt(pos);

    if (!index.isValid()) return;

    int linkId = index.data(Qt::UserRole).toInt();
    if (linkId <= 0) return;

    QMenu menu(this);
    QAction *openAct = menu.addAction(QStringLiteral("\U0001F310 \u6253\u5f00\u94fe\u63a5"));
    QAction *editAct = menu.addAction(QStringLiteral("\u270f \u7f16\u8f91"));
    menu.addSeparator();
    QAction *delAct = menu.addAction(QStringLiteral("\U0001F5D1 \u5220\u9664"));

    QAction *chosen = menu.exec(viewIdx == 0
        ? m_listView->viewport()->mapToGlobal(pos)
        : m_cardView->viewport()->mapToGlobal(pos));

    if (chosen == openAct) {
        auto opt = m_linkRepo->getById(linkId);
        if (opt.has_value()) {
            Link l = opt.value();
            l.visitCount++;
            l.lastVisitedAt = QDateTime::currentDateTime();
            m_linkRepo->update(l);
            PlatformUtils::openInBrowser(l.url);
        }
    } else if (chosen == editAct) {
        onDoubleClicked(linkId);
    } else if (chosen == delAct) {
        m_linkRepo->remove(linkId);
        refreshLinks();
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
