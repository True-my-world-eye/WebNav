// SettingsDialog.h — 偏好设置对话框
// 包含：主题切换、默认视图配配置、同步设置等

#pragma once
#include <QDialog>
#include <QComboBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    void setupUi();
    void applyTheme(const QString &key);
    void accept() override;

    QComboBox *m_themeCombo;        // 主题选择
};

