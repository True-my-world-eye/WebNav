#include "MainWindow.h"
#include "LinkEditDialog.h"
#include "SettingsDialog.h"
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
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QTextEdit>
#include "models/LinkField.h"
#include "models/Tag.h"
#include "services/BookmarkImporter.h"
#include "services/BookmarkExporter.h"

// ── 构造函数 ──────────────────────────────────────────────────
// 初始化主窗口，创建所有 UI 组件并连接信号槽
// @param linkRepo   链接仓库（依赖注入）
// @param folderRepo 文件夹仓库（依赖注入）
// @param tagRepo    标签仓库（依赖注入）
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

    // 失效链接筛选
    connect(m_sidebar, &Sidebar::brokenLinksRequested, this, [this]() {
        m_filterFolderId = -1;
        m_filterTagId = -1;
        m_filterKeyword.clear();
        buildLinkModel(m_linkRepo->getBroken());
    });

    refreshLinks();

    // 应用默认视图
    QSettings settings;
    QString defaultView = settings.value("defaultView", "list").toString();
    if (defaultView == "card" && !m_isCardView) {
        toggleView();
    }
}

MainWindow::~MainWindow() = default;

// ── 工具栏设置 ──────────────────────────────────────────────
// 创建工具栏按钮：搜索、新建、视图切换、编辑、打开、删除、排序、设置
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

    // \u2500\u2500 \u83dc\u5355\u680f \u2500\u2500
    auto *fileMenu = menuBar()->addMenu(QStringLiteral("\u6587\u4ef6"));
    auto *importAct = fileMenu->addAction(QStringLiteral("[\u5bfc\u5165] \u5bfc\u5165\u4e66\u7b7e..."));
    connect(importAct, &QAction::triggered, this, [this]() { onImportBookmarks(); });
    auto *exportAct = fileMenu->addAction(QStringLiteral("[\u5bfc\u51fa] \u5bfc\u51fa\u4e66\u7b7e..."));
    connect(exportAct, &QAction::triggered, this, [this]() { onExportBookmarks(); });

    auto *helpMenu = menuBar()->addMenu(QStringLiteral("\u5e2e\u52a9"));
    auto *helpAct = helpMenu->addAction(QStringLiteral("[?] \u4f7f\u7528\u8bf4\u660e"));
    connect(helpAct, &QAction::triggered, this, &MainWindow::openHelp);

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

// ── 状态栏设置 ──────────────────────────────────────────────
// 初始化状态栏显示
void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(QStringLiteral("\u51c6\u5907\u5c31\u7eea"));
}

// ── 窗口关闭事件 ──────────────────────────────────────────────
// 如果系统托盘可用，最小化到托盘而不是退出
void MainWindow::closeEvent(QCloseEvent *event)
{
    // \u6700\u5c0f\u5316\u5230\u6258\u76d8\uff0c\u4e0d\u9000\u51fa\uff08\u5982\u679c\u6709\u6258\u76d8\uff09
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

// ── 快捷键设置 ──────────────────────────────────────────────
// 注册全局快捷键：Ctrl+N 新建、Ctrl+F 搜索、Ctrl+1/2 视图切换、Delete 删除
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

// ── 构建链接模型 ──────────────────────────────────────────────
// 将链接数据转换为 QStandardItemModel，用于列表和卡片视图显示
// 包含：标题、URL、文件夹、标签、备注五列
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

    if (!m_faviconService) {
        m_faviconService = new FaviconService(this);
        connect(m_faviconService, &FaviconService::faviconReady, this,
            [this](const QString &pageUrl, const QString &localPath) {
            if (!m_linkModel || localPath.isEmpty()) return;
            QPixmap pix(localPath);
            if (pix.isNull()) return;
            for (int i = 0; i < m_linkModel->rowCount(); i++) {
                QString url = m_linkModel->item(i, 1) ? m_linkModel->item(i, 1)->text() : QString();
                if (url == pageUrl) {
                    m_linkModel->item(i, 0)->setIcon(QIcon(pix));
                    break;
                }
            }
        });
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

        auto *titleItem = makeItem(link.title.isEmpty() ? link.url : link.title);
        if (m_faviconService && !link.url.isEmpty()) {
            QString cached = m_faviconService->getCachedFavicon(link.url);
            if (!cached.isEmpty()) {
                QPixmap pix(cached);
                if (!pix.isNull())
                    titleItem->setIcon(QIcon(pix));
            } else {
                m_faviconService->fetchFavicon(link.url, [](const QString &){});
            }
        }
        m_linkModel->setItem(i, 0, titleItem);
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

// ── 更新状态栏 ──────────────────────────────────────────────
// 显示链接总数和选中数量
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

// ── 刷新链接列表 ──────────────────────────────────────────────
// 清除所有筛选条件，显示所有链接
void MainWindow::refreshLinks()
{
    m_filterFolderId = -1;
    m_filterTagId = -1;
    m_filterKeyword.clear();
    buildLinkModel(m_linkRepo->getAll());
}

// ── 搜索处理 ──────────────────────────────────────────────
// 根据关键词搜索链接，支持标题、URL、描述匹配
void MainWindow::onSearch(const QString &keyword)
{
    m_filterKeyword = keyword;
    if (keyword.isEmpty()) { applyFilters(); return; }
    buildLinkModel(m_linkRepo->search(keyword));
}

// ── 应用筛选条件 ──────────────────────────────────────────────
// 根据当前筛选状态（文件夹、标签）刷新链接列表
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

// ── 新建链接 ──────────────────────────────────────────────
// 打开编辑对话框，创建新链接并保存到数据库
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

// ── 视图切换 ──────────────────────────────────────────────
// 在列表视图和卡片视图之间切换
void MainWindow::toggleView()
{
    m_isCardView = !m_isCardView;
    m_viewStack->setCurrentIndex(m_isCardView ? 1 : 0);
    if (m_viewAction)
        m_viewAction->setText(m_isCardView
            ? QStringLiteral("\U0001F5BC \u5361\u7247")
            : QStringLiteral("\u5217\u8868"));
}

// ── 辅助方法 ──────────────────────────────────────────────────

// 获取当前选中的链接 ID
// 根据当前视图（列表或卡片）获取选中项
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

// 获取所有选中的链接 ID（支持多选）
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

// ── 双击编辑 ──────────────────────────────────────────────
// 双击列表项或卡片时，打开编辑对话框
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

// ── 编辑链接 ──────────────────────────────────────────────
// 通过工具栏编辑按钮触发
void MainWindow::onEditLink()
{
    int linkId = selectedLinkId();
    if (linkId <= 0) {
        statusBar()->showMessage(QStringLiteral("\u8bf7\u5148\u9009\u62e9\u4e00\u6761\u94fe\u63a5"), 3000);
        return;
    }
    onDoubleClicked(linkId);
}

// ── 打开链接 ──────────────────────────────────────────────
// 在系统默认浏览器中打开链接，并更新访问统计
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

// ── 删除链接 ──────────────────────────────────────────────
// 删除选中的链接（支持批量删除），显示确认对话框
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

// ── 右键菜单 ──────────────────────────────────────────────
// 显示上下文菜单，支持单条和批量操作
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

// ── 保存链接顺序 ──────────────────────────────────────────────
// 将当前列表中的链接顺序保存到数据库
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

// ── 移动链接位置 ──────────────────────────────────────────────
// @param direction  -1=上移, 1=下移, 0=置顶
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

// ── 批量操作 ──────────────────────────────────────────────────

// 批量打开链接
// 在系统浏览器中逐个打开选中的链接
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

// 批量打标签
// 弹出输入框，为所有选中的链接添加同一个标签
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

// 批量移动文件夹
// 弹出文件夹选择对话框，将所有选中的链接移动到目标文件夹
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

// ── 导入导出 ──────────────────────────────────────────────────

// 导入书签
// 支持 Chrome/Firefox/Edge 的 HTML 书签格式
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

// 导出书签
// 支持 CSV、HTML、Markdown 三种格式
void MainWindow::onExportBookmarks()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出书签"),
        QStringLiteral(""),
        QStringLiteral("CSV 文件 (*.csv);;HTML 书签 (*.html);;Markdown (*.md);;所有文件 (*)"));
    if (filePath.isEmpty()) return;

    BookmarkExporter exporter;
    auto links = m_linkRepo->getAll();
    auto folders = m_folderRepo ? m_folderRepo->getAll() : QVector<Folder>();

    bool ok = false;
    if (filePath.endsWith(".csv")) {
        // CSV 导出（直接在这里写，简单可靠）
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << "标题,URL,文件夹,标签,备注\n";
            QMap<int, QString> folderNames;
            for (const auto &f : folders) folderNames[f.id] = f.name;
            for (const auto &link : links) {
                QStringList tagNames;
                if (m_tagRepo) {
                    for (const auto &t : m_tagRepo->getTagsForLink(link.id))
                        tagNames << t.name;
                }
                // CSV 转义：含逗号或引号的字段用双引号包裹
                auto csvEscape = [](const QString &s) -> QString {
                    if (s.contains(',') || s.contains('"') || s.contains('\n'))
                        return "\"" + QString(s).replace("\"", "\"\"") + "\"";
                    return s;
                };
                out << csvEscape(link.title.isEmpty() ? link.url : link.title) << ","
                    << csvEscape(link.url) << ","
                    << csvEscape(folderNames.value(link.folderId)) << ","
                    << csvEscape(tagNames.join("; ")) << ","
                    << csvEscape(link.notes.left(100)) << "\n";
            }
            file.close();
            ok = true;
        }
    } else if (filePath.endsWith(".md"))
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

// ── 文件夹操作 ──────────────────────────────────────────────

// 新建文件夹
// 弹出输入框，创建新的文件夹
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

// 重命名文件夹
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

// 删除文件夹
// 显示确认对话框，删除文件夹及其所有子文件夹
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

// ── 标签操作 ──────────────────────────────────────────────────

// 删除标签
// 显示确认对话框，删除标签并从所有链接中移除
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

// ── 设置 ──────────────────────────────────────────────────────

// 打开设置对话框
// 保存设置后重新应用主题
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

// ── 帮助 ──────────────────────────────────────────────────────

// 打开使用说明对话框
// 显示 HTML 格式的帮助文档
void MainWindow::openHelp()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("使用说明 - WebNav"));
    dlg.resize(500, 500);
    auto *layout = new QVBoxLayout(&dlg);

    auto *text = new QTextEdit(&dlg);
    text->setReadOnly(true);
    text->setHtml(QStringLiteral(
        "<h2>WebNav - 桌面网页链接管理器</h2>"
        "<hr>"
        "<h3>基本操作</h3>"
        "<ul>"
        "<li><b>新建链接</b>：工具栏 [+新建] 按钮 或 Ctrl+N</li>"
        "<li><b>编辑链接</b>：双击数据行，或选中后点工具栏 [✏编辑]</li>"
        "<li><b>打开链接</b>：选中后点工具栏 [🌐打开]</li>"
        "<li><b>删除链接</b>：选中后按 Delete 键，或右键 → 删除</li>"
        "</ul>"
        "<h3>视图切换</h3>"
        "<ul>"
        "<li><b>列表视图</b>：Ctrl+1，以表格形式展示链接</li>"
        "<li><b>卡片视图</b>：Ctrl+2，以卡片网格展示（含网站图标、域名、标签）</li>"
        "</ul>"
        "<h3>排序功能</h3>"
        "<ul>"
        "<li><b>拖拽排序</b>：在列表视图中直接拖拽行到两行之间调整顺序</li>"
        "<li><b>工具栏排序</b>：选中链接后使用 [↑上移] [↓下移] [↥置顶] 按钮</li>"
        "</ul>"
        "<h3>文件夹与标签</h3>"
        "<ul>"
        "<li><b>新建文件夹/标签</b>：点击侧边栏标题旁的 [+] 按钮</li>"
        "<li><b>筛选</b>：点击文件夹或标签筛选链接，再次点击取消筛选</li>"
        "<li><b>文件夹右键</b>：新建子文件夹 / 重命名 / 删除</li>"
        "<li><b>标签右键</b>：删除标签</li>"
        "<li><b>失效链接</b>：点击侧边栏 [⚠] 按钮筛选失效链接</li>"
        "</ul>"
        "<h3>批量操作</h3>"
        "<ul>"
        "<li>按住 Ctrl 多选链接，右键弹出批量菜单</li>"
        "<li>支持：批量打开、批量打标签、批量移动文件夹、批量删除</li>"
        "</ul>"
        "<h3>复制功能</h3>"
        "<ul>"
        "<li>右键链接 → [复制链接]：复制 URL 到剪贴板</li>"
        "<li>右键链接 → [复制标题+链接]：同时复制标题和 URL</li>"
        "</ul>"
        "<h3>导入导出</h3>"
        "<ul>"
        "<li><b>导入</b>：文件 → 导入书签，支持 Chrome/Firefox/Edge HTML 书签</li>"
        "<li><b>导出</b>：文件 → 导出书签，可导出为 CSV 或 HTML 格式</li>"
        "</ul>"
        "<h3>快捷键</h3>"
        "<ul>"
        "<li>Ctrl+N 新建 | Ctrl+F 搜索 | Delete 删除</li>"
        "<li>Ctrl+1 列表 | Ctrl+2 卡片</li>"
        "<li>双击编辑 | 右键菜单</li>"
        "</ul>"
        "<hr>"
        "<p align='center'>"
        "<b>开发者</b>：Truemwe<br>"
        "<b>联系邮箱</b>：<a href='mailto:hwzhang0722@163.com'>hwzhang0722@163.com</a><br><br>"
        "希望 WebNav 能为你的日常网页收藏与管理带来便利！"
        "</p>"
    ));
    layout->addWidget(text);

    auto *closeBtn = new QPushButton(QStringLiteral("关闭"), &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg.exec();
}
