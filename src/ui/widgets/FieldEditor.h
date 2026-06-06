// FieldEditor.h — 附加字段编辑器
// 用于编辑链接关联的账号/密码/邮箱/电话/自定义字段
// 支持预设字段一键添加、自定义字段扩展、密码加密存储

#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVector>
#include "models/LinkField.h"

class FieldEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FieldEditor(QWidget *parent = nullptr);

    // 设置已有字段列表（编辑模式时调用）
    void setFields(const QVector<LinkField> &fields);

    // 获取所有字段（用于保存）
    QVector<LinkField> fields() const;

signals:
    void fieldsChanged();

private slots:
    // 添加预设字段（账号/密码/邮箱/电话）
    void addPresetField(const QString &fieldKey);

    // 添加自定义字段
    void addCustomField();

    // 移除字段
    void removeField(QWidget *fieldRow);

private:
    // 创建单行字段编辑控件
    QWidget *createFieldRow(const QString &key, const QString &value,
                            bool isPassword, int typeIndex);

    QVBoxLayout *m_fieldsLayout;         // 字段列表布局
    QPushButton *m_addCustomBtn;         // "添加自定义字段"按钮
    QVector<LinkField> m_currentFields;  // 当前字段列表

    // 预设字段配置
    struct PresetField {
        QString key;        // field_key
        QString label;      // 显示名
        bool isPassword;    // 是否加密
    };
    static QVector<PresetField> presetFields();
};

