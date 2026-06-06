// Sidebar.h — 左侧导航栏
// 显示智能列表、文件夹树和标签云，提供导航筛选功能

#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QLabel>

class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);

    // 刷新所有数据
    void refresh();

signals:
    // 用户选择了"所有链接"
    void allLinksRequested();

    // 用户选择了"最近添加"
    void recentLinksRequested();

    // 用户选择了"频繁访问"
    void frequentLinksRequested();

    // 用户选择了某个文件夹
    void folderSelected(int folderId);

    // 用户选择了某个标签
    void tagSelected(int tagId);

private:
    // 初始化智能列表（所有/最近/频繁/失效）
    void setupSmartList();

    // 初始化标签列表
    void setupTagList();

    QTreeWidget *m_smartList;       // 智能列表导航
    QListWidget *m_tagList;         // 标签列表
    QLabel *m_tagLabel;             // "标签"标题
};
