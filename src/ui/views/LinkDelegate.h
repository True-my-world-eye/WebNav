// LinkDelegate.h — 链接列表/卡片自定义绘制代理

#pragma once
#include <QStyledItemDelegate>

class LinkDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit LinkDelegate(QObject *parent = nullptr);

    // 列表视图：每行显示 favicon + 标题 + URL + 标签 + 日期
    // 卡片视图：显示缩略图 + 标题 + 域名 + 标签
    enum ViewMode { ListView, CardView };

    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

    // 自定义绘制
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    ViewMode m_viewMode = ListView;
};

