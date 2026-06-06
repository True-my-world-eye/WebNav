// SettingsDialog.cpp

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle(QStringLiteral("\u8bbe\u7f6e"));
    setFixedSize(400, 300);
}

void SettingsDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // ── 外观设置 ──
    auto *appearanceGroup = new QGroupBox(QStringLiteral("\u5916\u89c2"), this);
    auto *appearanceLayout = new QFormLayout(appearanceGroup);

    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem(QStringLiteral("\u968f\u7cfb\u7edf"), "system");
    m_themeCombo->addItem(QStringLiteral("\u6d45\u8272"), "light");
    m_themeCombo->addItem(QStringLiteral("\u6df1\u8272"), "dark");
    appearanceLayout->addRow(QStringLiteral("\u4e3b\u9898:"), m_themeCombo);

    mainLayout->addWidget(appearanceGroup);

    mainLayout->addStretch();

    // 关闭按钮
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto *closeBtn = new QPushButton(QStringLiteral("\u5173\u95ed"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);
}

