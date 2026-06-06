// AboutDialog.cpp

#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("\u5173\u4e8e WebNav"));
    setFixedSize(360, 200);

    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("WebNav v1.0.0"), this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(title);

    layout->addSpacing(8);

    auto *desc = new QLabel(QStringLiteral(
        "\u57fa\u4e8e C++ / Qt 6 \u7684\u684c\u9762\u7f51\u9875\u94fe\u63a5\u7ba1\u7406\u5668\n"
        "\u53cc\u89c6\u56fe\u53ef\u5207\u6362 \u00B7 \u94fe\u63a5\u8d26\u5bc6\u5b58\u50a8 \u00B7 \u53ef\u6269\u5c55\u4e91\u7aef\u540c\u6b65"),
        this);
    desc->setAlignment(Qt::AlignCenter);
    desc->setWordWrap(true);
    layout->addWidget(desc);

    layout->addStretch();

    auto *closeBtn = new QPushButton(QStringLiteral("\u5173\u95ed"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);
}
