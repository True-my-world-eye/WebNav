// LinkPreviewPanel.cpp

#include "LinkPreviewPanel.h"
#include <QLabel>
#include <QVBoxLayout>

LinkPreviewPanel::LinkPreviewPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *placeholder = new QLabel(QStringLiteral("\u9875\u9762\u9884\u89c8\u9762\u677f\n(Phase 2 \u5b9e\u73b0)"), this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: #888;");
    layout->addWidget(placeholder);

    setMinimumWidth(300);
    hide();     // 默认隐藏
}

void LinkPreviewPanel::previewUrl(const QString &url)
{
    // Phase 2: 使用 Qt WebEngine 加载 URL
    Q_UNUSED(url)
    show();
}

void LinkPreviewPanel::clearPreview()
{
    hide();
}

