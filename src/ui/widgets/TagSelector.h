#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>
#include <QSet>
#include "models/Tag.h"

class TagSelector : public QWidget
{
    Q_OBJECT
public:
    explicit TagSelector(QWidget *parent = nullptr);
    void setAvailableTags(const QVector<Tag> &tags);
    QVector<int> selectedTagIds() const { return m_selectedIds.values(); }
    void setSelectedTagIds(const QVector<int> &ids);
signals:
    void tagsChanged();
    void createNewTag(const QString &name);
private:
    QLineEdit *m_input;
    QCompleter *m_completer;
    QStringListModel *m_completerModel;
    QVector<Tag> m_availableTags;
    QSet<int> m_selectedIds;
};
