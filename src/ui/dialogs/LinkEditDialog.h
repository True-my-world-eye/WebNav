#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include "models/Link.h"
#include "models/Folder.h"
#include "models/Tag.h"
#include "models/LinkField.h"
#include "TagSelector.h"
#include "FieldEditor.h"

class LinkEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LinkEditDialog(QWidget *parent = nullptr);
    void setLink(const Link &link);
    Link link() const;
    QVector<int> selectedTagIds() const { return m_tagSelector->selectedTagIds(); }
    QVector<LinkField> linkFields() const { return m_fieldEditor->fields(); }
    void setFolders(const QVector<Folder> &folders);
    void setTags(const QVector<Tag> &tags);
    TagSelector *tagSelector() const { return m_tagSelector; }
    FieldEditor *fieldEditor() const { return m_fieldEditor; }
signals:
    void createNewTag(const QString &name);
private slots:
    void onSave();
private:
    void setupUi();
    QLineEdit   *m_urlInput;
    QLineEdit   *m_titleInput;
    QComboBox   *m_folderCombo;
    TagSelector *m_tagSelector;
    QTextEdit   *m_notesEdit;
    FieldEditor *m_fieldEditor;
    QPushButton *m_saveBtn;
    Link m_currentLink;
    bool m_editMode = false;
};
