// LinkPreviewPanel.h — 网页预览面板（Phase 2 实现）
// 使用 Qt WebEngine 内嵌浏览器预览链接页面

#pragma once
#include <QWidget>
#include <QUrl>

class LinkPreviewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LinkPreviewPanel(QWidget *parent = nullptr);

    // 加载并预览指定 URL
    void previewUrl(const QString &url);

    // 清除当前预览
    void clearPreview();
};

