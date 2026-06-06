// LinkCardView.h — 链接卡片视图
// 以卡片网格形式展示链接，每张卡片包含缩略图/标题/域名/标签

#pragma once
#include <QListView>
#include <QStandardItemModel>

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

