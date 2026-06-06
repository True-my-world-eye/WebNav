// LinkCardView.cpp

#include "LinkCardView.h"
#include <QScrollArea>

LinkCardView::LinkCardView(QWidget *parent)
    : QListView(parent)
{
    setViewMode(QListView::IconMode);
    setResizeMode(QListView::Adjust);
    setGridSize(QSize(220, 180));
    setSpacing(8);
    setWordWrap(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        if (index.isValid())
        {
            int linkId = index.data(Qt::UserRole).toInt();
            emit linkDoubleClicked(linkId);
        }
    });
}

void LinkCardView::setLinkData(QStandardItemModel *model)
{
    m_model = model;
    setModel(m_model);
}

