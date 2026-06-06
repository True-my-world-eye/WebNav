// SearchBar.h — 搜索输入框
// 支持实时搜索、防抖、快捷键聚焦

#pragma once
#include <QLineEdit>
#include <QTimer>

class SearchBar : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);

signals:
    // 用户输入完成后触发搜索（300ms 防抖）
    void searchTriggered(const QString &keyword);

private:
    QTimer *m_debounceTimer;    // 防抖定时器
};

