// SettingsDialog.cpp

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QApplication>
#include <QFile>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle(QStringLiteral("设置"));
    setFixedSize(380, 250);

    // load saved theme
    QSettings settings;
    QString savedTheme = settings.value("theme", "dark").toString();
    for (int i = 0; i < m_themeCombo->count(); i++) {
        if (m_themeCombo->itemData(i).toString() == savedTheme) {
            m_themeCombo->setCurrentIndex(i);
            break;
        }
    }
    // apply theme on startup
    applyTheme(savedTheme);
}

void SettingsDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // appearance
    auto *appearanceGroup = new QGroupBox(QStringLiteral("外观"), this);
    auto *appearanceLayout = new QFormLayout(appearanceGroup);

    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem(QStringLiteral("深色"), "dark");
    m_themeCombo->addItem(QStringLiteral("浅色"), "light");
    appearanceLayout->addRow(QStringLiteral("主题:"), m_themeCombo);

    connect(m_themeCombo, &QComboBox::currentIndexChanged, this, [this]() {
        applyTheme(m_themeCombo->currentData().toString());
    });

    mainLayout->addWidget(appearanceGroup);

    mainLayout->addStretch();

    // close button
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto *closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);
}

void SettingsDialog::applyTheme(const QString &key)
{
    QString qssFile;
    if (key == "light")
        qssFile = QStringLiteral(":/themes/light.qss");
    else
        qssFile = QStringLiteral(":/themes/dark.qss");

    QFile file(qssFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}

void SettingsDialog::accept()
{
    QSettings settings;
    settings.setValue("theme", m_themeCombo->currentData().toString());
    QDialog::accept();
}
