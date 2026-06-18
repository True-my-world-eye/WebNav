// GlobalHotkey.cpp

#include "GlobalHotkey.h"
#include <QDebug>

GlobalHotkey::GlobalHotkey(QObject *parent)
    : QObject(parent)
{
}

bool GlobalHotkey::registerHotkey(const QKeySequence &keys)
{
    m_keys = keys;
    // TODO: Phase 2 实现
    // Windows: RegisterHotKey
    // macOS:   CGEvent
    // Linux:   XCB
    qInfo() << "[Hotkey] \u5168\u5c40\u5feb\u6377\u952e\u5c06\u5728 Phase 2 \u5b9e\u73b0:"
            << keys.toString();
    return false;
}

bool GlobalHotkey::unregisterHotkey()
{
    if (!m_registered) return true;
    m_registered = false;
    return true;
}

bool GlobalHotkey::isRegistered() const
{
    return m_registered;
}
