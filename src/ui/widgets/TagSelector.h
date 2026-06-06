// TagSelector.h — 标签选择器（带可视标签气泡 + 流式布局）

#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>
#include <QSet>
#include <QPushButton>
#include <QLayout>
#include <QList>
#include <QStyle>
#include "models/Tag.h"

// ── 流式布局（自动折行） ──────────────────────────
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout() override;
    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;

    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    QList<QLayoutItem *> m_itemList;
    int m_hSpace;
    int m_vSpace;
};

// ── 标签选择器 ──────────────────────────────────
class TagSelector : public QWidget
{
    Q_OBJECT
public:
    explicit TagSelector(QWidget *parent = nullptr);

    void setAvailableTags(const QVector<Tag> &tags);
    QVector<int> selectedTagIds() const { return m_selectedIds.values().toVector(); }
    void setSelectedTagIds(const QVector<int> &ids);
    void addSelectedTagId(int id);

signals:
    void tagsChanged();
    void createNewTag(const QString &name);

private slots:
    void handleInputConfirmed();

private:
    bool selectTagByName(const QString &name);
    void rebuildTagChips();

    QLineEdit *m_input;
    QCompleter *m_completer;
    QStringListModel *m_completerModel;
    FlowLayout *m_selectedLayout;
    QVector<Tag> m_availableTags;
    QSet<int> m_selectedIds;
};
