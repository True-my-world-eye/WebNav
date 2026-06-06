#include "TagSelector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>

TagSelector::TagSelector(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(new QLabel(QStringLiteral("\u6807\u7b7e:"), this));

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("\u8f93\u5165\u6807\u7b7e\u540d\u79f0\u9009\u62e9\u6216\u65b0\u5efa..."));
    m_input->setClearButtonEnabled(true);
    layout->addWidget(m_input);

    m_completerModel = new QStringListModel(this);
    m_completer = new QCompleter(m_completerModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_input->setCompleter(m_completer);

    connect(m_input, &QLineEdit::returnPressed, this, [this]() {
        QString text = m_input->text().trimmed();
        if (text.isEmpty()) return;
        // 检查是否匹配已有标签
        for (const auto &tag : m_availableTags) {
            if (tag.name == text && !m_selectedIds.contains(tag.id)) {
                m_selectedIds.insert(tag.id);
                emit tagsChanged();
                m_input->clear();
                return;
            }
        }
        // 未匹配：创建新标签
        emit createNewTag(text);
        m_input->clear();
    });
}

void TagSelector::setAvailableTags(const QVector<Tag> &tags)
{
    m_availableTags = tags;
    QStringList names;
    for (const auto &t : tags) names << t.name;
    m_completerModel->setStringList(names);
}

void TagSelector::setSelectedTagIds(const QVector<int> &ids)
{
    m_selectedIds.clear();
    for (int id : ids) m_selectedIds.insert(id);
}
