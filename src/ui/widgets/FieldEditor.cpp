// FieldEditor.cpp — 附加字段编辑器实现

#include "FieldEditor.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>

FieldEditor::FieldEditor(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    auto *header = new QLabel(QStringLiteral("\u9644\u52a0\u6570\u636e"), this);
    header->setObjectName("sectionTitle");
    mainLayout->addWidget(header);

    // 预设字段按钮行
    auto *presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(4);
    for (const auto &pf : presetFields())
    {
        auto *btn = new QPushButton(QStringLiteral("+%1").arg(pf.label), this);
        btn->setFixedHeight(24);
        connect(btn, &QPushButton::clicked, this, [this, key = pf.key]() {
            addPresetField(key);
        });
        presetLayout->addWidget(btn);
    }
    presetLayout->addStretch();
    mainLayout->addLayout(presetLayout);

    m_fieldsLayout = new QVBoxLayout();
    m_fieldsLayout->setSpacing(4);
    mainLayout->addLayout(m_fieldsLayout);

    m_addCustomBtn = new QPushButton(QStringLiteral("+ \u6dfb\u52a0\u81ea\u5b9a\u4e49\u5b57\u6bb5"), this);
    m_addCustomBtn->setFixedHeight(28);
    connect(m_addCustomBtn, &QPushButton::clicked, this, &FieldEditor::addCustomField);
    mainLayout->addWidget(m_addCustomBtn);
}

QVector<FieldEditor::PresetField> FieldEditor::presetFields()
{
    return {
        {"account",  QStringLiteral("\u8d26\u53f7"), false},
        {"password", QStringLiteral("\u5bc6\u7801"), true},
        {"email",    QStringLiteral("\u90ae\u7bb1"), false},
        {"phone",    QStringLiteral("\u7535\u8bdd"), false},
    };
}

void FieldEditor::setFields(const QVector<LinkField> &fields)
{
    QLayoutItem *item;
    while ((item = m_fieldsLayout->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    m_currentFields = fields;
    for (const auto &field : fields)
    {
        auto *row = createFieldRow(field.fieldKey, field.fieldValue,
                                    field.isPassword, field.fieldType);
        m_fieldsLayout->addWidget(row);
    }
}

QVector<LinkField> FieldEditor::fields() const
{
    return m_currentFields;
}

void FieldEditor::addPresetField(const QString &fieldKey)
{
    for (const auto &field : m_currentFields)
    {
        if (field.fieldKey == fieldKey)
            return;
    }
    for (const auto &pf : presetFields())
    {
        if (pf.key == fieldKey)
        {
            LinkField field;
            field.fieldKey = fieldKey;
            field.isPassword = pf.isPassword;
            field.fieldType = pf.isPassword ? 1 : 0;
            field.sortOrder = m_currentFields.size();
            m_currentFields.append(field);
            auto *row = createFieldRow(fieldKey, "", pf.isPassword, field.fieldType);
            m_fieldsLayout->addWidget(row);
            emit fieldsChanged();
            return;
        }
    }
}

void FieldEditor::addCustomField()
{
    bool ok;
    QString fieldName = QInputDialog::getText(this,
        QStringLiteral("\u65b0\u5efa\u81ea\u5b9a\u4e49\u5b57\u6bb5"),
        QStringLiteral("\u5b57\u6bb5\u540d\u79f0:"),
        QLineEdit::Normal, "", &ok);
    if (!ok || fieldName.trimmed().isEmpty()) return;

    auto reply = QMessageBox::question(this,
        QStringLiteral("\u52a0\u5bc6\u8bbe\u7f6e"),
        QStringLiteral("\u662f\u5426\u9700\u8981\u52a0\u5bc6\u5b58\u50a8\u8be5\u5b57\u6bb5\u7684\u503c\uff1f"),
        QMessageBox::Yes | QMessageBox::No);
    bool encrypt = (reply == QMessageBox::Yes);

    LinkField field;
    field.fieldKey = fieldName.trimmed();
    field.isPassword = encrypt;
    field.fieldType = encrypt ? 1 : 0;
    field.sortOrder = m_currentFields.size();
    m_currentFields.append(field);
    auto *row = createFieldRow(fieldName.trimmed(), "", encrypt, field.fieldType);
    m_fieldsLayout->addWidget(row);
    emit fieldsChanged();
}

void FieldEditor::removeField(QWidget *fieldRow)
{
    int index = m_fieldsLayout->indexOf(fieldRow);
    if (index >= 0 && index < m_currentFields.size())
    {
        m_currentFields.removeAt(index);
        m_fieldsLayout->removeWidget(fieldRow);
        fieldRow->deleteLater();
        emit fieldsChanged();
    }
}

QWidget *FieldEditor::createFieldRow(const QString &key, const QString &value,
                                      bool isPassword, int typeIndex)
{
    Q_UNUSED(typeIndex)
    auto *row = new QWidget(this);
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *label = new QLabel(key, row);
    label->setFixedWidth(60);
    layout->addWidget(label);

    auto *input = new QLineEdit(value, row);
    if (isPassword)
    {
        input->setEchoMode(QLineEdit::Password);
        auto *toggleBtn = new QPushButton(QStringLiteral("\U0001F441"), row);
        toggleBtn->setFixedSize(24, 24);
        toggleBtn->setToolTip(QStringLiteral("\u663e\u793a/\u9690\u85cf\u5bc6\u7801"));
        toggleBtn->setCheckable(true);
        connect(toggleBtn, &QPushButton::toggled, this, [input](bool checked) {
            input->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        });
        layout->addWidget(toggleBtn);
    }
    input->setPlaceholderText(QStringLiteral("\u8f93\u5165%1").arg(key));
    layout->addWidget(input);

    auto *removeBtn = new QPushButton(QStringLiteral("\u00D7"), row);
    removeBtn->setFixedSize(24, 24);
    connect(removeBtn, &QPushButton::clicked, this, [this, row]() {
        removeField(row);
    });
    layout->addWidget(removeBtn);

    connect(input, &QLineEdit::textChanged, this, [this, input, key]() {
        for (auto &field : m_currentFields)
        {
            if (field.fieldKey == key)
            {
                field.fieldValue = input->text();
                break;
            }
        }
    });
    return row;
}
