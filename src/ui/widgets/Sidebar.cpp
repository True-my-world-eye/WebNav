// Sidebar.cpp

#include "Sidebar.h"
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    setFixedWidth(220);
    setObjectName("sidebar");

    // 智能列表
    setupSmartList();
    layout->addWidget(m_smartList);

    layout->addSpacing(8);

    // 标签标题
    m_tagLabel = new QLabel(QStringLiteral("\u6807\u7b7e"), this);
    m_tagLabel->setObjectName("sectionTitle");
    layout->addWidget(m_tagLabel);

    // 标签列表
    setupTagList();
    layout->addWidget(m_tagList);

    layout->addStretch();
}

void Sidebar::setupSmartList()
{
    m_smartList = new QTreeWidget(this);
    m_smartList->setHeaderHidden(true);
    m_smartList->setRootIsDecorated(false);
    m_smartList->setIndentation(12);

    // 添加导航项
    auto *allItem = new QTreeWidgetItem(m_smartList);
    allItem->setText(0, QStringLiteral("\U0001F4DA \u6240\u6709\u94fe\u63a5"));

    auto *recentItem = new QTreeWidgetItem(m_smartList);
    recentItem->setText(0, QStringLiteral("\u23F0 \u6700\u8fd1\u6dfb\u52a0"));

    auto *freqItem = new QTreeWidgetItem(m_smartList);
    freqItem->setText(0, QStringLiteral("\U0001F525 \u9891\u7e41\u8bbf\u95ee"));

    auto *brokenItem = new QTreeWidgetItem(m_smartList);
    brokenItem->setText(0, QStringLiteral("\u274C \u5931\u6548\u94fe\u63a5"));

    // 连接信号：点击导航项发出对应的请求信号
    connect(m_smartList, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        QString text = item->text(0);
        if (text.contains(QStringLiteral("\u6240\u6709")))         // 所有链接
            emit allLinksRequested();
        else if (text.contains(QStringLiteral("\u6700\u8fd1")))     // 最近添加
            emit recentLinksRequested();
        else if (text.contains(QStringLiteral("\u9891\u7e41")))     // 频繁访问
            emit frequentLinksRequested();
        else                                                         // 失效链接
            emit allLinksRequested();  // TODO: emit brokenLinksRequested
    });
}

void Sidebar::setupTagList()
{
    m_tagList = new QListWidget(this);
    m_tagList->setObjectName("tagList");

    connect(m_tagList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int tagId = item->data(Qt::UserRole).toInt();
        emit tagSelected(tagId);
    });
}

void Sidebar::refresh()
{
    // TODO: Phase 1 实现完成后，从数据库读取标签列表并填充 m_tagList
    // 从 SqliteTagRepository 获取所有标签
    // 为每个标签创建带颜色的 QListWidgetItem
}

