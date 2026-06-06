// LinkDelegate.cpp

#include "LinkDelegate.h"
#include <QPainter>
#include <QApplication>

LinkDelegate::LinkDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void LinkDelegate::setViewMode(ViewMode mode)
{
    m_viewMode = mode;
}

LinkDelegate::ViewMode LinkDelegate::viewMode() const
{
    return m_viewMode;
}

void LinkDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    // 使用默认绘制
    QStyledItemDelegate::paint(painter, option, index);
}

QSize LinkDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (m_viewMode == CardView)
        return QSize(200, 160);

    return QStyledItemDelegate::sizeHint(option, index);
}
