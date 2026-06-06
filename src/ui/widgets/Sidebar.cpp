#include "Sidebar.h"
#include "database/interfaces/IFolderRepository.h"
#include "database/interfaces/ITagRepository.h"
#include "models/Folder.h"
#include "models/Tag.h"
#include "ColorUtils.h"
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);
    layout->setSpacing(2);
    setFixedWidth(220);
    setObjectName("sidebar");

    setupSmartList();
    layout->addWidget(m_smartList);

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

void Sidebar::setupSmartList()
{
    m_smartList = new QTreeWidget(this);
    m_smartList->setHeaderHidden(true);
    m_smartList->setRootIsDecorated(false);
    m_smartList->setIndentation(12);
    m_smartList->setMaximumHeight(130);

    auto addItem = [&](const QString &text) {
        auto *item = new QTreeWidgetItem(m_smartList);
        item->setText(0, text);
        return item;
    };
    addItem(QStringLiteral("\U0001F4DA \u6240\u6709\u94fe\u63a5"));
    addItem(QStringLiteral("\u23F0 \u6700\u8fd1\u6dfb\u52a0"));
    addItem(QStringLiteral("\U0001F525 \u9891\u7e41\u8bbf\u95ee"));

    connect(m_smartList, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        QString t = item->text(0);
        if (t.contains("\u6240\u6709")) emit allLinksRequested();
        else if (t.contains("\u6700\u8fd1")) emit recentLinksRequested();
        else if (t.contains("\u9891\u7e41")) emit frequentLinksRequested();
    });
}

void Sidebar::setupFolderTree()
{
    m_folderTree = new QTreeWidget(this);
    m_folderTree->setHeaderHidden(true);
    m_folderTree->setAnimated(true);
    m_folderTree->setIndentation(14);

    connect(m_folderTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        int fid = item->data(0, Qt::UserRole).toInt();
        emit folderSelected(fid);
    });
}

void Sidebar::setupTagList()
{
    m_tagList = new QListWidget(this);
    m_tagList->setObjectName("tagList");
    m_tagList->setMaximumHeight(200);

    connect(m_tagList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int tid = item->data(Qt::UserRole).toInt();
        emit tagSelected(tid);
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
