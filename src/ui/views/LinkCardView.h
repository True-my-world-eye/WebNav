// LinkCardView.h — 链接卡片视图（增强版）
// 以卡片网格形式展示链接，每张卡片包含标题/域名/标签气泡

#pragma once
#include <QListView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class LinkCardView : public QListView
{
    Q_OBJECT

public:
    explicit LinkCardView(QWidget *parent = nullptr);

    void setLinkData(QStandardItemModel *model);

signals:
    void linkDoubleClicked(int linkId);

private:
    QStandardItemModel *m_model = nullptr;
};

// ── 卡片代理 ──────────────────────────────────────
class LinkCardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LinkCardDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};
