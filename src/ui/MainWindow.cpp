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
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QApplication>
#include "models/LinkField.h"
#include "models/Tag.h"
#include "services/BookmarkImporter.h"
#include "services/BookmarkExporter.h"

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

    // ── 列表视图设置：只读 + 右键菜单 + 拖拽排序 ──
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    // 拖拽已在 LinkListView 构造函数中设置，这里不需要重复

    // ── 卡片视图设置：只读 + 右键菜单 ──
    m_cardView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cardView->setContextMenuPolicy(Qt::CustomContextMenu);

    // ── 信号连接 ──
    connect(m_listView, &LinkListView::linkDoubleClicked, this, &MainWindow::onDoubleClicked);
    connect(m_cardView, &LinkCardView::linkDoubleClicked, this, &MainWindow::onDoubleClicked);
    connect(m_listView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(m_cardView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    // 选中行变化 → 更新状态栏
    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateStatusBar);
    connect(m_cardView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateStatusBar);

    // ── 拖拽排序：由 LinkListView 手动管理拖拽，完成后持久化 ──
    connect(m_listView, &LinkListView::linkMoveRequested, this, [this](int /*linkId*/, int /*insertRow*/) {
        // 用拖拽后的 model 顺序保存 sort_order
        if (!m_linkModel) return;
        saveLinkOrder();
    });

    connect(m_sidebar, &Sidebar::folderSelected, this, [this](int fid) {
        m_filterFolderId = fid;
        applyFilters();
    });
    connect(m_sidebar, &Sidebar::tagSelected, this, [this](int tagId) {
        m_filterTagId = tagId;
        applyFilters();
    });
    connect(m_sidebar, &Sidebar::folderStructureChanged, this, &MainWindow::refreshLinks);
    connect(m_sidebar, &Sidebar::folderNewRequested, this, &MainWindow::onNewFolder);
    connect(m_sidebar, &Sidebar::folderRenameRequested, this, &MainWindow::onRenameFolder);
    connect(m_sidebar, &Sidebar::folderDeleteRequested, this, &MainWindow::onDeleteFolder);
    connect(m_sidebar, &Sidebar::tagDeleteRequested, this, &MainWindow::onDeleteTag);

    refreshLinks();

    // 应用默认视图
    QSettings settings;
    QString defaultView = settings.value("defaultView", "list").toString();
    if (defaultView == "card" && !m_isCardView) {
        toggleView();
    }
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

    // \u2500\u2500 \u6587\u4ef6\u83dc\u5355 \u2500\u2500
    auto *fileMenu = menuBar()->addMenu(QStringLiteral("\u6587\u4ef6"));
    auto *importAct = fileMenu->addAction(QStringLiteral("[\u5bfc\u5165] \u5bfc\u5165\u4e66\u7b7e..."));
    connect(importAct, &QAction::triggered, this, [this]() { onImportBookmarks(); });
    auto *exportAct = fileMenu->addAction(QStringLiteral("[\u5bfc\u51fa] \u5bfc\u51fa\u4e66\u7b7e..."));
    connect(exportAct, &QAction::triggered, this, [this]() { onExportBookmarks(); });
    fileMenu->addSeparator();
    auto *aboutAct = fileMenu->addAction(QStringLiteral("\u5173\u4e8e WebNav"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::openAbout);

    // \u2500\u2500 \u6392\u5e8f\u64cd\u4f5c \u2500\u2500
    auto *moveUpAct = tb->addAction(QStringLiteral("\u2191 \u4e0a\u79fb"));
    moveUpAct->setToolTip(QStringLiteral("\u5c06\u9009\u4e2d\u94fe\u63a5\u4e0a\u79fb\u4e00\u884c"));
    connect(moveUpAct, &QAction::triggered, this, [this]() { moveSelectedLink(-1); });

    auto *moveDownAct = tb->addAction(QStringLiteral("\u2193 \u4e0b\u79fb"));
    moveDownAct->setToolTip(QStringLiteral("\u5c06\u9009\u4e2d\u94fe\u63a5\u4e0b\u79fb\u4e00\u884c"));
    connect(moveDownAct, &QAction::triggered, this, [this]() { moveSelectedLink(1); });

    auto *moveTopAct = tb->addAction(QStringLiteral("\u21a5 \u7f6e\u9876"));
    moveTopAct->setToolTip(QStringLiteral("\u5c06\u9009\u4e2d\u94fe\u63a5\u7f6e\u9876"));
    connect(moveTopAct, &QAction::triggered, this, [this]() { moveSelectedLink(0); });

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

    auto *delSC = new QShortcut(QKeySequence("Delete"), this);
    connect(delSC, &QShortcut::activated, this, &MainWindow::onDeleteLink);
}

void MainWindow::buildLinkModel(const QVector<Link> &links)
{
    m_isRebuildingModel = true;

    delete m_linkModel;
    // 5 列：标题 / URL / 文件夹 / 标签（去掉时间列）
    m_linkModel = new QStandardItemModel(links.size(), 5, this);
    m_linkModel->setHorizontalHeaderLabels({
        QStringLiteral("\u6807\u9898"), QStringLiteral("URL"),
        QStringLiteral("\u6587\u4ef6\u5939"), QStringLiteral("\u6807\u7b7e"),
        QStringLiteral("\u5907\u6ce8")});

    QMap<int, QString> folderNames;
    if (m_folderRepo) {
        for (const auto &f : m_folderRepo->getAll())
            folderNames[f.id] = f.name;
    }

    for (int i = 0; i < links.size(); i++)
    {
        const auto &link = links[i];

        // 每列都存入 linkId 到 Qt::UserRole，确保点击任意位置都能识别所属链接
        auto makeItem = [&](const QString &text) -> QStandardItem* {
            auto *item = new QStandardItem(text);
            item->setData(link.id, Qt::UserRole);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setToolTip(text);
            return item;
        };

        m_linkModel->setItem(i, 0, makeItem(link.title.isEmpty() ? link.url : link.title));
        m_linkModel->setItem(i, 1, makeItem(link.url));

        QString folderName = folderNames.value(link.folderId);
        m_linkModel->setItem(i, 2, makeItem(folderName));

        QStringList tagNames;
        if (m_tagRepo) {
            for (const auto &t : m_tagRepo->getTagsForLink(link.id))
                tagNames << t.name;
        }
        m_linkModel->setItem(i, 3, makeItem(tagNames.join(", ")));
        m_linkModel->setItem(i, 4, makeItem(link.notes.left(60)));
    }

    m_listView->setLinkData(m_linkModel);
    m_cardView->setLinkData(m_linkModel);

    // set default widths for Interactive columns
    if (m_linkModel->columnCount() >= 4) {
        m_listView->setColumnWidth(0, 200);
        m_listView->setColumnWidth(1, 220);
        m_listView->setColumnWidth(2, 100);
        m_listView->setColumnWidth(3, 120);
    }

    m_isRebuildingModel = false;

    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    if (!m_linkModel) {
        statusBar()->showMessage(QStringLiteral("\u51c6\u5907\u5c31\u7eea"));
        return;
    }

    int total = m_linkModel->rowCount();
    if (total == 0) {
        statusBar()->showMessage(QStringLiteral("\u6682\u65e0\u94fe\u63a5"));
        return;
    }

    int viewIdx = m_viewStack->currentIndex();
    QItemSelectionModel *sel = (viewIdx == 0)
        ? m_listView->selectionModel()
        : m_cardView->selectionModel();
    int selected = (sel && sel->hasSelection()) ? sel->selectedRows().size() : 0;

    QString msg = QStringLiteral("\u5171 %1 \u6761\u94fe\u63a5").arg(total);
    if (selected > 0)
        msg += QStringLiteral(" | \u9009\u4e2d %1 \u6761").arg(selected);
    statusBar()->showMessage(msg);
}

void MainWindow::refreshLinks()
{
    m_filterFolderId = -1;
    m_filterTagId = -1;
    m_filterKeyword.clear();
    buildLinkModel(m_linkRepo->getAll());
}

void MainWindow::onSearch(const QString &keyword)
{
    m_filterKeyword = keyword;
    if (keyword.isEmpty()) { applyFilters(); return; }
    buildLinkModel(m_linkRepo->search(keyword));
}

void MainWindow::applyFilters()
{
    m_filterKeyword.clear();
    m_searchBar->clear();
    if (m_filterFolderId >= 0 && m_filterTagId >= 0)
        buildLinkModel(m_linkRepo->getByFolderAndTag(m_filterFolderId, m_filterTagId));
    else if (m_filterFolderId >= 0)
        buildLinkModel(m_linkRepo->getByFolder(m_filterFolderId));
    else if (m_filterTagId >= 0)
        buildLinkModel(m_linkRepo->getByTag(m_filterTagId));
    else
        buildLinkModel(m_linkRepo->getAll());
}

void MainWindow::onNewLink()
{
    LinkEditDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u65b0\u5efa\u94fe\u63a5"));
    dialog.setFolders(m_folderRepo ? m_folderRepo->getAll() : QVector<Folder>());
    dialog.setTags(m_tagRepo ? m_tagRepo->getAll() : QVector<Tag>());

    connect(&dialog, &LinkEditDialog::createNewTag, this, [this, &dialog](const QString &name) {
        Tag t; t.name = name;
        int newId = m_tagRepo->insert(t);
        if (newId > 0) {
            dialog.tagSelector()->addSelectedTagId(newId);
            dialog.setTags(m_tagRepo->getAll());  // 刷新补全列表
        }
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
    QItemSelectionModel *sel = (viewIdx == 0)
        ? m_listView->selectionModel()
        : m_cardView->selectionModel();
    if (sel && sel->hasSelection())
        return sel->currentIndex().data(Qt::UserRole).toInt();
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
    auto opt = m_linkRepo->getById(linkId);
    if (!opt.has_value()) return;
    Link link = opt.value();

    LinkEditDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u7f16\u8f91\u94fe\u63a5"));
    dialog.setFolders(m_folderRepo->getAll());
    dialog.setTags(m_tagRepo->getAll());
    dialog.setLink(link);
    dialog.setLinkTime(link.createdAt, link.updatedAt);
    dialog.fieldEditor()->setFields(m_linkRepo->getLinkFields(linkId));

    QVector<int> tagIds;
    for (const auto &t : m_tagRepo->getTagsForLink(linkId))
        tagIds.append(t.id);
    dialog.tagSelector()->setSelectedTagIds(tagIds);

    connect(&dialog, &LinkEditDialog::createNewTag, this, [this, &dialog](const QString &name) {
        Tag t; t.name = name;
        int newId = m_tagRepo->insert(t);
        if (newId > 0) {
            dialog.tagSelector()->addSelectedTagId(newId);
            dialog.setTags(m_tagRepo->getAll());  // 刷新补全列表
        }
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        Link updated = dialog.link();
        updated.id = linkId;
        updated.sortOrder = link.sortOrder;
        updated.createdAt = link.createdAt;
        updated.visitCount = link.visitCount;
        updated.lastVisitedAt = link.lastVisitedAt;
        m_linkRepo->update(updated);

        if (m_tagRepo) {
            for (const auto &t : m_tagRepo->getTagsForLink(linkId))
                m_tagRepo->removeTagFromLink(linkId, t.id);
            for (int tagId : dialog.selectedTagIds())
                m_tagRepo->addTagToLink(linkId, tagId);
        }

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
    int viewIdx = m_viewStack->currentIndex();
    QAbstractItemView *view = (viewIdx == 0)
        ? static_cast<QAbstractItemView*>(m_listView)
        : static_cast<QAbstractItemView*>(m_cardView);
    QModelIndex index = view->indexAt(pos);

    if (!index.isValid()) return;

    int linkId = index.data(Qt::UserRole).toInt();
    if (linkId <= 0) return;

    QMenu menu(this);

    // \u2500\u2500 \u5355\u6761\u64cd\u4f5c \u2500\u2500
    QAction *openAct = menu.addAction(QStringLiteral("\U0001F310 \u6253\u5f00\u94fe\u63a5"));
    QAction *editAct = menu.addAction(QStringLiteral("\u270f \u7f16\u8f91"));
    menu.addSeparator();

    // \u2500\u2500 \u6279\u91cf\u64cd\u4f5c\uff08\u5bf9\u9009\u4e2d\u5168\u90e8\u6709\u6548\uff09 \u2500\u2500
    QVector<int> allSelected = selectedLinkIds();
    if (allSelected.size() > 1) {
        QAction *batchOpenAct = menu.addAction(QStringLiteral("\U0001F310 \u6279\u91cf\u6253\u5f00(%1\u4e2a)").arg(allSelected.size()));
        QAction *batchTagAct = menu.addAction(QStringLiteral("\U0001F3F7 \u6279\u91cf\u6253\u6807\u7b7e"));
        QAction *batchFolderAct = menu.addAction(QStringLiteral("\U0001F4C1 \u6279\u91cf\u79fb\u52a8\u6587\u4ef6\u5939"));
        menu.addSeparator();
        QAction *delAct = menu.addAction(QStringLiteral("\U0001F5D1 \u5220\u9664(%1\u4e2a)").arg(allSelected.size()));

        QAction *chosen = menu.exec(view->viewport()->mapToGlobal(pos));

        if (chosen == openAct) {
            onOpenLink();
        } else if (chosen == editAct) {
            onDoubleClicked(linkId);
        } else if (chosen == batchOpenAct) {
            batchOpen(allSelected);
        } else if (chosen == batchTagAct) {
            batchTag(allSelected);
        } else if (chosen == batchFolderAct) {
            batchMoveFolder(allSelected);
        } else if (chosen == delAct) {
            onDeleteLink();
        }
    } else {
        menu.addSeparator();
        QAction *copyUrlAct = menu.addAction(QStringLiteral("\U0001F4CB \u590d\u5236\u94fe\u63a5"));
        QAction *copyTitleUrlAct = menu.addAction(QStringLiteral("\U0001F4CB \u590d\u5236\u6807\u9898+\u94fe\u63a5"));
        menu.addSeparator();
        QAction *delAct = menu.addAction(QStringLiteral("\U0001F5D1 \u5220\u9664"));

        QAction *chosen = menu.exec(view->viewport()->mapToGlobal(pos));

        if (chosen == openAct) {
            onOpenLink();
        } else if (chosen == editAct) {
            onDoubleClicked(linkId);
        } else if (chosen == copyUrlAct) {
            auto opt = m_linkRepo->getById(linkId);
            if (opt.has_value()) {
                QClipboard *cb = QApplication::clipboard();
                cb->setText(opt->url);
                statusBar()->showMessage(QStringLiteral("\u94fe\u63a5\u5df2\u590d\u5236"), 2000);
            }
        } else if (chosen == copyTitleUrlAct) {
            auto opt = m_linkRepo->getById(linkId);
            if (opt.has_value()) {
                QClipboard *cb = QApplication::clipboard();
                cb->setText(opt->title + "\n" + opt->url);
                statusBar()->showMessage(QStringLiteral("\u6807\u9898+\u94fe\u63a5\u5df2\u590d\u5236"), 2000);
            }
        } else if (chosen == delAct) {
            m_linkRepo->remove(linkId);
            refreshLinks();
        }
    }
}

void MainWindow::saveLinkOrder()
{
    if (!m_linkModel || m_isRebuildingModel) return;
    QVector<QPair<int,int>> orders;
    for (int i = 0; i < m_linkModel->rowCount(); i++)
    {
        int linkId = m_linkModel->item(i, 0)->data(Qt::UserRole).toInt();
        if (linkId > 0) orders.append({linkId, i});
    }
    if (!orders.isEmpty())
        m_linkRepo->reorderLinks(orders);
}

void MainWindow::moveSelectedLink(int direction)
{
    int linkId = selectedLinkId();
    if (linkId <= 0 || !m_linkModel) return;

    // 找到该 link 的当前行
    int currentRow = -1;
    for (int i = 0; i < m_linkModel->rowCount(); i++) {
        if (m_linkModel->item(i, 0)->data(Qt::UserRole).toInt() == linkId) {
            currentRow = i;
            break;
        }
    }
    if (currentRow < 0) return;

    int rowCount = m_linkModel->rowCount();
    int newRow = currentRow;

    if (direction == -1 && currentRow > 0) {
        newRow = currentRow - 1;  // 上移
    } else if (direction == 1 && currentRow < rowCount - 1) {
        newRow = currentRow + 1;  // 下移
    } else if (direction == 0 && currentRow > 0) {
        newRow = 0;  // 置顶
    } else {
        return;  // 已经是边界
    }

    // 交换两行的 item（移动行数据）
    int cols = m_linkModel->columnCount();
    // 收集源行
    QVector<QStandardItem*> srcItems;
    srcItems.reserve(cols);
    for (int c = 0; c < cols; c++) {
        srcItems.append(m_linkModel->takeItem(currentRow, c));
    }
    m_linkModel->removeRow(currentRow);

    // 如果 newRow > currentRow，因删行需 -1
    int actualTarget = (newRow > currentRow) ? newRow - 1 : newRow;
    if (actualTarget < 0) actualTarget = 0;
    if (actualTarget > m_linkModel->rowCount()) actualTarget = m_linkModel->rowCount();

    // 在目标位置插入
    m_linkModel->insertRow(actualTarget);
    for (int c = 0; c < cols; c++) {
        m_linkModel->setItem(actualTarget, c, srcItems[c]);
    }

    // 选中移动后的行
    QModelIndex newIdx = m_linkModel->index(actualTarget, 0);
    m_listView->selectionModel()->setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);

    saveLinkOrder();
}

void MainWindow::batchOpen(const QVector<int> &ids)
{
    if (ids.isEmpty()) return;
    for (int id : ids) {
        auto opt = m_linkRepo->getById(id);
        if (opt.has_value()) {
            Link l = opt.value();
            l.visitCount++;
            l.lastVisitedAt = QDateTime::currentDateTime();
            m_linkRepo->update(l);
            PlatformUtils::openInBrowser(l.url);
        }
    }
    statusBar()->showMessage(
        QStringLiteral("已打开 %1 个链接").arg(ids.size()), 3000);
}

void MainWindow::batchTag(const QVector<int> &ids)
{
    if (ids.isEmpty()) return;
    bool ok;
    QString tagName = QInputDialog::getText(this,
        QStringLiteral("批量打标签"),
        QStringLiteral("输入标签名称（已有标签将被打上，不存在则新建）:"),
        QLineEdit::Normal, "", &ok);
    if (!ok || tagName.trimmed().isEmpty()) return;

    // 查找或创建标签
    auto existing = m_tagRepo->getByName(tagName.trimmed());
    int tagId;
    if (existing.has_value()) {
        tagId = existing->id;
    } else {
        Tag t; t.name = tagName.trimmed();
        tagId = m_tagRepo->insert(t);
    }
    if (tagId <= 0) return;

    for (int linkId : ids)
        m_tagRepo->addTagToLink(linkId, tagId);

    statusBar()->showMessage(
        QStringLiteral("已为 %1 个链接打上标签 \"%2\"").arg(ids.size()).arg(tagName.trimmed()), 3000);
    m_sidebar->refresh();
    refreshLinks();
}

void MainWindow::batchMoveFolder(const QVector<int> &ids)
{
    if (ids.isEmpty()) return;
    // 弹出文件夹选择
    QStringList items;
    QMap<int, QString> folderMap;
    if (m_folderRepo) {
        int idx = 0;
        items << QStringLiteral("未分类");
        folderMap[-1] = QStringLiteral("未分类");
        for (const auto &f : m_folderRepo->getAll()) {
            items << f.name;
            folderMap[f.id] = f.name;
        }
    }
    bool ok;
    QString selected = QInputDialog::getItem(this,
        QStringLiteral("批量移动文件夹"),
        QStringLiteral("选择目标文件夹:"),
        items, 0, false, &ok);
    if (!ok || selected.isEmpty()) return;

    // 找到选中项的 id
    int targetFolderId = -1;
    for (auto it = folderMap.begin(); it != folderMap.end(); ++it) {
        if (it.value() == selected) {
            targetFolderId = it.key();
            break;
        }
    }

    for (int linkId : ids) {
        auto opt = m_linkRepo->getById(linkId);
        if (opt.has_value()) {
            Link l = opt.value();
            l.folderId = targetFolderId;
            m_linkRepo->update(l);
        }
    }

    statusBar()->showMessage(
        QStringLiteral("已移动 %1 个链接到 \"%2\"").arg(ids.size()).arg(selected), 3000);
    refreshLinks();
}

void MainWindow::onImportBookmarks()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        QStringLiteral("导入书签"),
        QString(),
        QStringLiteral("HTML 书签文件 (*.html);;所有文件 (*)"));
    if (filePath.isEmpty()) return;

    BookmarkImporter importer;
    ImportResult result = importer.importFromFile(filePath);

    if (!result.errorMessage.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("导入失败"), result.errorMessage);
        return;
    }

    // 将解析出的文件夹和链接写入数据库
    if (m_folderRepo && !importer.parsedFolders().isEmpty()) {
        for (const auto &f : importer.parsedFolders())
            m_folderRepo->insert(f);
    }
    if (m_linkRepo && !importer.parsedLinks().isEmpty()) {
        int imported = 0;
        for (const auto &link : importer.parsedLinks()) {
            int id = m_linkRepo->insert(link);
            if (id > 0) imported++;
        }
        statusBar()->showMessage(
            QStringLiteral("导入完成：新增 %1 个链接，跳过 %2 个重复，创建 %3 个文件夹")
                .arg(result.importedCount).arg(result.skippedCount).arg(result.folderCount), 5000);
    }

    m_sidebar->refresh();
    refreshLinks();
}

void MainWindow::onExportBookmarks()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出书签"),
        QStringLiteral("bookmarks.html"),
        QStringLiteral("HTML 书签文件 (*.html);;Markdown (*.md);;所有文件 (*)"));
    if (filePath.isEmpty()) return;

    BookmarkExporter exporter;
    auto links = m_linkRepo->getAll();
    auto folders = m_folderRepo ? m_folderRepo->getAll() : QVector<Folder>();

    bool ok = false;
    if (filePath.endsWith(".md"))
        ok = exporter.exportToMarkdown(filePath, links, folders);
    else
        ok = exporter.exportToFile(filePath, links, folders);

    if (ok)
        statusBar()->showMessage(
            QStringLiteral("已导出 %1 条链接到 %2").arg(links.size()).arg(filePath), 5000);
    else
        QMessageBox::warning(this, QStringLiteral("导出失败"),
            QStringLiteral("无法写入文件，请检查路径是否有写权限。"));
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
        refreshLinks();
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

void MainWindow::onDeleteTag(int tagId)
{
    QString tagName;
    auto opt = m_tagRepo->getById(tagId);
    if (opt.has_value()) tagName = opt->name;

    auto ret = QMessageBox::question(this,
        QStringLiteral("\u5220\u9664\u6807\u7b7e"),
        QStringLiteral("\u786e\u5b9a\u8981\u5220\u9664\u6807\u7b7e\u201c%1\u201d\u5417\uff1f\u8be5\u6807\u7b7e\u5c06\u4ece\u6240\u6709\u94fe\u63a5\u4e2d\u79fb\u9664\u3002").arg(tagName),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_tagRepo->remove(tagId);
        m_sidebar->refresh();
        refreshLinks();
    }
}

void MainWindow::openSettings()
{
    SettingsDialog d(this);
    if (d.exec() == QDialog::Accepted) {
        QSettings settings;
        // 重新应用主题
        QString theme = settings.value("theme", "dark").toString();
        QString qssFile = (theme == "light")
            ? QStringLiteral(":/themes/light.qss")
            : QStringLiteral(":/themes/dark.qss");
        QFile file(qssFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qApp->setStyleSheet(file.readAll());
            file.close();
        }
    }
}

void MainWindow::openAbout()
{
    AboutDialog d(this); d.exec();
}
