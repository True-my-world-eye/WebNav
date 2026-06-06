#include "Sidebar.h"
#include "database/interfaces/IFolderRepository.h"
#include "database/interfaces/ITagRepository.h"
#include "models/Folder.h"
#include "models/Tag.h"
#include "ColorUtils.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>

Sidebar::Sidebar(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);
    layout->setSpacing(2);
    setFixedWidth(220);
    setObjectName("sidebar");

    auto *folderTitle = new QLabel(QStringLiteral("\U0001F4C1 \u6587\u4ef6\u5939"), this);
    folderTitle->setObjectName("sectionTitle");
    layout->addWidget(folderTitle);

    setupFolderTree();
    layout->addWidget(m_folderTree);

    auto *tagTitle = new QLabel(QStringLiteral("\U0001F3F7 \u6807\u7b7e"), this);
    tagTitle->setObjectName("sectionTitle");
    layout->addWidget(tagTitle);

    setupTagList();
    layout->addWidget(m_tagList);
    layout->addStretch();
}

void Sidebar::setRepositories(IFolderRepository *folderRepo, ITagRepository *tagRepo)
{
    m_folderRepo = folderRepo;
    m_tagRepo = tagRepo;
    refresh();
}

void Sidebar::setupFolderTree()
{
    m_folderTree = new QTreeWidget(this);
    m_folderTree->setHeaderHidden(true);
    m_folderTree->setAnimated(true);
    m_folderTree->setIndentation(14);
    m_folderTree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_folderTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        int fid = item->data(0, Qt::UserRole).toInt();
        emit folderSelected(fid);
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

void Sidebar::setupTagList()
{
    m_tagList = new QListWidget(this);
    m_tagList->setObjectName("tagList");
    m_tagList->setMaximumHeight(200);
    m_tagList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_tagList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int tid = item->data(Qt::UserRole).toInt();
        emit tagSelected(tid);
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

void Sidebar::refresh()
{
    // ── 刷新文件夹树 ──
    m_folderTree->clear();
    if (m_folderRepo)
    {
        auto roots = m_folderRepo->getRootFolders();
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
        addFolders(-1, nullptr);
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
}
