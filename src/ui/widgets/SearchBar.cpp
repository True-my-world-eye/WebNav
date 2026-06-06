// SearchBar.cpp

#include "SearchBar.h"

SearchBar::SearchBar(QWidget *parent)
    : QLineEdit(parent)
    , m_debounceTimer(new QTimer(this))
{
    setPlaceholderText(QStringLiteral("\U0001F50D \u641c\u7d22\u94fe\u63a5..."));
    setClearButtonEnabled(true);
    setObjectName("searchBar");

    // 防抖：用户停止输入 300ms 后触发搜索
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(300);

    connect(this, &QLineEdit::textChanged, this, [this]() {
        m_debounceTimer->start();
    });

    connect(m_debounceTimer, &QTimer::timeout, this, [this]() {
        emit searchTriggered(text().trimmed());
    });
}
