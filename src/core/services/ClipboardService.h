// ClipboardService.h — 剪贴板监控服务（Phase 2 实现）
// 监听系统剪贴板变化，检测到 URL 时触发收藏提示

#pragma once
#include <QObject>

class ClipboardService : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardService(QObject *parent = nullptr);

    // 启动/停止剪贴板监控
    void start();
    void stop();

    // 是否正在监控
    bool isRunning() const;

signals:
    // 检测到 URL 时发出信号
    void urlDetected(const QString &url);

private:
    bool m_running = false;
};
