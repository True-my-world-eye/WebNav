// Sidebar.cpp — 侧边栏组件实现
// 左侧导航栏，包含文件夹树和标签列表
// 支持：文件夹/标签筛选、失效链接筛选、右键菜单操作

#include "Sidebar.h"
#include "database/interfaces/IFolderRepository.h"
#include "database/interfaces/ITagRepository.h"
#include "models/Folder.h"
#include "models/Tag.h"
#include "ColorUtils.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeWidgetItemIterator>

// ── 构造函数 ──────────────────────────────────────────────────
// 初始化侧边栏布局，创建文件夹区域和标签区域
Sidebar::Sidebar(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);
    layout->setSpacing(2);
    setFixedWidth(220);
    setObjectName("sidebar");

    auto *folderHeader = new QHBoxLayout();
    folderHeader->setContentsMargins(0,0,0,0);
    auto *folderTitle = new QLabel(QStringLiteral("\U0001F4C1 \u6587\u4ef6\u5939"), this);
    folderTitle->setObjectName("sectionTitle");

    auto *brokenBtn = new QPushButton(QStringLiteral("\u26a0"), this);
    brokenBtn->setFixedSize(20, 20);
    brokenBtn->setObjectName("sidebarAddBtn");
    brokenBtn->setToolTip(QStringLiteral("\u67e5\u770b\u5931\u6548\u94fe\u63a5"));
    brokenBtn->setCheckable(true);
    m_brokenBtn = brokenBtn;
    connect(brokenBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) {
            m_showingBroken = true;
            // \u53d6\u6d88\u5176\u4ed6\u9009\u4e2d
            m_selectedFolderId = -1;
            m_selectedTagId = -1;
            m_folderTree->clearSelection();
            m_tagList->clearSelection();
            emit brokenLinksRequested();
        } else {
            m_showingBroken = false;
            emit folderSelected(-1);
        }
    });

    folderHeader->addWidget(folderTitle);
    folderHeader->addStretch();
    folderHeader->addWidget(brokenBtn);
    auto *folderAddBtn = new QPushButton(QStringLiteral("+"), this);
    folderAddBtn->setFixedSize(20, 20);
    folderAddBtn->setObjectName("sidebarAddBtn");
    folderAddBtn->setToolTip(QStringLiteral("\u65b0\u5efa\u6587\u4ef6\u5939"));
    connect(folderAddBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("\u65b0\u5efa\u6587\u4ef6\u5939"),
            QStringLiteral("\u6587\u4ef6\u5939\u540d\u79f0:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            Folder f; f.name = name.trimmed();
            if (m_folderRepo) { m_folderRepo->insert(f); refresh(); emit folderStructureChanged(); }
        }
    });
    folderHeader->addWidget(folderAddBtn);
    layout->addLayout(folderHeader);

    setupFolderTree();
    layout->addWidget(m_folderTree);

    auto *tagHeader = new QHBoxLayout();
    tagHeader->setContentsMargins(0,0,0,0);
    auto *tagTitle = new QLabel(QStringLiteral("\U0001F3F7 \u6807\u7b7e"), this);
    tagTitle->setObjectName("sectionTitle");
    tagHeader->addWidget(tagTitle);
    tagHeader->addStretch();
    auto *tagAddBtn = new QPushButton(QStringLiteral("+"), this);
    tagAddBtn->setFixedSize(20, 20);
    tagAddBtn->setObjectName("sidebarAddBtn");
    tagAddBtn->setToolTip(QStringLiteral("\u65b0\u5efa\u6807\u7b7e"));
    connect(tagAddBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("\u65b0\u5efa\u6807\u7b7e"),
            QStringLiteral("\u6807\u7b7e\u540d\u79f0:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            Tag t; t.name = name.trimmed();
            if (m_tagRepo) { m_tagRepo->insert(t); refresh(); emit folderStructureChanged(); }
        }
    });
    tagHeader->addWidget(tagAddBtn);
    layout->addLayout(tagHeader);

    setupTagList();
    layout->addWidget(m_tagList);
    layout->addStretch();
}

// ── 依赖注入 ──────────────────────────────────────────────────
// 设置仓库实例，并刷新侧边栏内容
void Sidebar::setRepositories(IFolderRepository *folderRepo, ITagRepository *tagRepo)
{
    m_folderRepo = folderRepo;
    m_tagRepo = tagRepo;
    refresh();
}

// ── 文件夹树设置 ──────────────────────────────────────────────
// 创建文件夹树控件，设置交互行为：
// - 单击：选中/取消选中文件夹
// - 右键：上下文菜单（新建子文件夹、重命名、删除）
void Sidebar::setupFolderTree()
{
    m_folderTree = new QTreeWidget(this);
    m_folderTree->setHeaderHidden(true);
    m_folderTree->setAnimated(true);
    m_folderTree->setIndentation(14);
    m_folderTree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_folderTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        int fid = item->data(0, Qt::UserRole).toInt();
        // 点击已选中的 = 取消选中
        if (fid == m_selectedFolderId) {
            m_selectedFolderId = -1;
            item->setSelected(false);
            emit folderSelected(-1);
        } else {
            m_selectedFolderId = fid;
            m_showingBroken = false;
            if (m_brokenBtn) m_brokenBtn->setChecked(false);
            // 取消标签的选中
            if (m_selectedTagId != -1) {
                m_selectedTagId = -1;
                for (int i = 0; i < m_tagList->count(); i++) {
                    auto *ti = m_tagList->item(i);
                    if (ti->data(Qt::UserRole).toInt() == m_selectedTagId)
                        ti->setSelected(false);
                }
            }
            emit folderSelected(fid);
        }
    });

    connect(m_folderTree, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QTreeWidgetItem *item = m_folderTree->itemAt(pos);
        QMenu menu(this);
        if (item) {
            int fid = item->data(0, Qt::UserRole).toInt();
            QAction *newAct = menu.addAction(QStringLiteral("\U0001F4C4 \u65b0\u5efa\u5b50\u6587\u4ef6\u5939"));
            QAction *renameAct = menu.addAction(QStringLiteral("\u270f \u91cd\u547d\u540d"));
            menu.addSeparator();
            QAction *delAct = menu.addAction(QStringLiteral("\U0001F5D1 \u5220\u9664"));

            QAction *chosen = menu.exec(m_folderTree->viewport()->mapToGlobal(pos));
            if (chosen == newAct)      emit folderNewRequested(fid);
            else if (chosen == renameAct) emit folderRenameRequested(fid);
            else if (chosen == delAct)   emit folderDeleteRequested(fid);
        } else {
            QAction *newAct = menu.addAction(QStringLiteral("\U0001F4C4 \u65b0\u5efa\u6587\u4ef6\u5939"));
            if (menu.exec(m_folderTree->viewport()->mapToGlobal(pos)) == newAct)
                emit folderNewRequested(-1);
        }
    });
}

// ── 标签列表设置 ──────────────────────────────────────────────
// 创建标签列表控件，设置交互行为：
// - 单击：选中/取消选中标签
// - 右键：上下文菜单（删除标签）
void Sidebar::setupTagList()
{
    m_tagList = new QListWidget(this);
    m_tagList->setObjectName("tagList");
    m_tagList->setMaximumHeight(200);
    m_tagList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_tagList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int tid = item->data(Qt::UserRole).toInt();
        // 点击已选中的 = 取消选中
        if (tid == m_selectedTagId) {
            m_selectedTagId = -1;
            item->setSelected(false);
            emit tagSelected(-1);
        } else {
            m_selectedTagId = tid;
            m_showingBroken = false;
            if (m_brokenBtn) m_brokenBtn->setChecked(false);
            // 取消文件夹的选中
            if (m_selectedFolderId != -1) {
                m_selectedFolderId = -1;
                // 清除文件夹树的选择
                QTreeWidgetItemIterator it(m_folderTree);
                while (*it) {
                    if ((*it)->data(0, Qt::UserRole).toInt() == m_selectedFolderId) {
                        (*it)->setSelected(false);
                        break;
                    }
                    ++it;
                }
            }
            emit tagSelected(tid);
        }
    });

    connect(m_tagList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = m_tagList->itemAt(pos);
        if (!item) return;
        int tid = item->data(Qt::UserRole).toInt();
        QMenu menu(this);
        QAction *delAct = menu.addAction(QStringLiteral("\U0001F5D1 \u5220\u9664\u6807\u7b7e"));
        if (menu.exec(m_tagList->viewport()->mapToGlobal(pos)) == delAct)
            emit tagDeleteRequested(tid);
    });
}

// ── 刷新侧边栏 ──────────────────────────────────────────────
// 重新从数据库加载文件夹树和标签列表
// 用于数据变更后刷新显示
void Sidebar::refresh()
{
    // ── 刷新文件夹树 ──
    m_folderTree->clear();
    if (m_folderRepo)
    {
        // 从根级文件夹开始递归构建（根文件夹 parent_id IS NULL）
        std::function<void(int, QTreeWidgetItem*)> addFolders;
        addFolders = [&](int parentId, QTreeWidgetItem *parent) {
            auto children = m_folderRepo->getByParent(parentId);
            for (const auto &f : children)
            {
                auto *item = parent
                    ? new QTreeWidgetItem(parent)
                    : new QTreeWidgetItem(m_folderTree);
                item->setText(0, f.name);
                item->setData(0, Qt::UserRole, f.id);
                addFolders(f.id, item);
            }
        };
        // 先获取根级文件夹（parent_id IS NULL），再以它们的 ID 递归
        auto roots = m_folderRepo->getRootFolders();
        for (const auto &f : roots)
        {
            auto *item = new QTreeWidgetItem(m_folderTree);
            item->setText(0, f.name);
            item->setData(0, Qt::UserRole, f.id);
            addFolders(f.id, item);
        }
    }

    // ── 刷新标签列表 ──
    m_tagList->clear();
    if (m_tagRepo)
    {
        auto tags = m_tagRepo->getAll();
        for (const auto &tag : tags)
        {
            auto *item = new QListWidgetItem(tag.name, m_tagList);
            item->setData(Qt::UserRole, tag.id);
            item->setForeground(QColor(tag.color));
        }
    }

    // 刷新后恢复失效按钮的选中状态
    if (m_brokenBtn)
        m_brokenBtn->setChecked(m_showingBroken);
}
