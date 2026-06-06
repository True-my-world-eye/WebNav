// TagSelector.h — 标签选择器
// 下拉式标签多选组件，支持搜索标签、创建新标签

#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>
#include <QVector>
#include "models/Tag.h"

class TagSelector : public QWidget
{
    Q_OBJECT

public:
    explicit TagSelector(QWidget *parent = nullptr);

    // 设置可用标签列表
    void setAvailableTags(const QVector<Tag> &tags);

    // 获取已选择的标签 ID 列表
    QVector<int> selectedTagIds() const;

    // 设置已选标签
    void setSelectedTagIds(const QVector<int> &tagIds);

signals:
    void tagsChanged();

private:
    QLineEdit *m_input;                     // 标签输入框
    QCompleter *m_completer;                // 自动补全
    QStringListModel *m_completerModel;     // 补全数据模型
    QVector<Tag> m_availableTags;           // 可用标签列表
};

