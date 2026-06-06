// ClipboardService.cpp

#include "ClipboardService.h"

ClipboardService::ClipboardService(QObject *parent)
    : QObject(parent)
{
}

void ClipboardService::start()
{
    m_running = true;
    // Phase 2 实现：使用 QClipboard 或 Windows 剪贴板 API 监听
}

void ClipboardService::stop()
{
    m_running = false;
}

bool ClipboardService::isRunning() const
{
    return m_running;
}
