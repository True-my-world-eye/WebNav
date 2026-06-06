// LinkEditDialog.h — 添加/编辑链接对话框
// 包含：URL/标题、文件夹选择、标签选择、附加字段编辑、备注

#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include "models/Link.h"
#include "models/Folder.h"
#include "models/Tag.h"
#include "TagSelector.h"
#include "FieldEditor.h"

class LinkEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LinkEditDialog(QWidget *parent = nullptr);

    // 编辑模式：传入已有链接预填表单
    void setLink(const Link &link);

    // 获取编辑后的链接
    Link link() const;

    // 设置可用数据
    void setFolders(const QVector<Folder> &folders);
    void setTags(const QVector<Tag> &tags);

private slots:
    // URL 输入后自动抓取标题
    void onUrlChanged(const QString &url);

    // 保存
    void onSave();

private:
    void setupUi();

    QLineEdit   *m_urlInput;          // URL 输入框
    QLineEdit   *m_titleInput;        // 标题输入框
    QComboBox   *m_folderCombo;       // 文件夹选择下拉框
    TagSelector *m_tagSelector;       // 标签选择器
    QTextEdit   *m_notesEdit;         // 备注编辑框
    FieldEditor *m_fieldEditor;       // 附加字段编辑器
    QPushButton *m_saveBtn;           // 保存按钮

    Link        m_currentLink;        // 当前正在编辑的链接
    bool        m_editMode = false;   // 是否为编辑模式
};

