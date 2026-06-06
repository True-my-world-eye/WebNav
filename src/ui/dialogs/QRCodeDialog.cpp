// QRCodeDialog.cpp

#include "QRCodeDialog.h"
#include <QVBoxLayout>
#include <QLabel>

QRCodeDialog::QRCodeDialog(const QString &url, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("QR \u7801\u5206\u4eab"));
    setFixedSize(300, 350);

    auto *layout = new QVBoxLayout(this);
    auto *label = new QLabel(QStringLiteral("Phase 2 \u5b9e\u73b0\n\n"), this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    auto *urlLabel = new QLabel(url, this);
    urlLabel->setWordWrap(true);
    urlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(urlLabel);
}

