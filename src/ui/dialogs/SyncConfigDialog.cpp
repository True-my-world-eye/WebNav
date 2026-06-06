// SyncConfigDialog.cpp

#include "SyncConfigDialog.h"
#include <QVBoxLayout>
#include <QLabel>

SyncConfigDialog::SyncConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("\u540c\u6b65\u914d\u7f6e"));
    setFixedSize(400, 250);

    auto *layout = new QVBoxLayout(this);
    auto *placeholder = new QLabel(QStringLiteral(
        "\u540c\u6b65\u529f\u80fd\u5c06\u5728 Phase 3 \u5b9e\u73b0\u3002\n\n"
        "\u652f\u6301\u90e8\u7f72\u5230 Alibaba Linux ECS\uff0c\n"
        "\u901a\u8fc7 Nginx \u53cd\u5411\u4ee3\u7406\u4e0d\u5f71\u54cd\u73b0\u6709 Hexo \u535a\u5ba2\u3002"),
        this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setWordWrap(true);
    layout->addWidget(placeholder);
}

