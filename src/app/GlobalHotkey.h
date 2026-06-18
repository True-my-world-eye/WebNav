// GlobalHotkey.h — 全局快捷键管理
// 注册系统级快捷键，支持后台运行时快速收藏

#pragma once
#include <QObject>
#include <QKeySequence>

class GlobalHotkey : public QObject
{
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject *parent = nullptr);

    // 注册全局快捷键
    bool registerHotkey(const QKeySequence &keys);

    // 注销全局快捷键
    bool unregisterHotkey();

    // 是否已注册
    bool isRegistered() const;

signals:
    // 全局快捷键触发信号
    void hotkeyTriggered();

private:
    bool m_registered = false;
    QKeySequence m_keys;
    int m_hotkeyId = 0;             // Windows RegisterHotKey 用 ID
};
