// TagSelector.cpp

#include "TagSelector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>

TagSelector::TagSelector(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *label = new QLabel(QStringLiteral("\u6807\u7b7e:"), this);
    layout->addWidget(label);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("\u8f93\u5165\u6807\u7b7e\u540d\u79f0\u9009\u62e9\u6216\u65b0\u5efa..."));
    m_input->setClearButtonEnabled(true);
    layout->addWidget(m_input);

    // 自动补全
    m_completerModel = new QStringListModel(this);
    m_completer = new QCompleter(m_completerModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_input->setCompleter(m_completer);

    connect(m_input, &QLineEdit::returnPressed, this, [this]() {
        // 用户按回车确认标签
        QString text = m_input->text().trimmed();
        if (!text.isEmpty())
        {
            // 检查是否为已有标签
            for (const auto &tag : m_availableTags)
            {
                if (tag.name == text)
                {
                    // 选中已有标签
                    emit tagsChanged();
                    m_input->clear();
                    return;
                }
            }
            // 新标签：后续创建逻辑在对话框层面处理
            emit tagsChanged();
            m_input->clear();
        }
    });
}

void TagSelector::setAvailableTags(const QVector<Tag> &tags)
{
    m_availableTags = tags;
    QStringList names;
    for (const auto &tag : tags)
        names << tag.name;
    m_completerModel->setStringList(names);
}

QVector<int> TagSelector::selectedTagIds() const
{
    // TODO: Phase 1 实现完成后，返回已选择的标签 ID
    return {};
}

void TagSelector::setSelectedTagIds(const QVector<int> &tagIds)
{
    Q_UNUSED(tagIds)
    // TODO: 设置已选标签
}
