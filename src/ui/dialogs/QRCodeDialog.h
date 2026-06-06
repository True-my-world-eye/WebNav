// QRCodeDialog.h — QR 码分享对话框（Phase 2 实现）

#pragma once
#include <QDialog>

class QRCodeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QRCodeDialog(const QString &url, QWidget *parent = nullptr);
};

