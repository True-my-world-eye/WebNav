// TagLabel.cpp

#include "TagLabel.h"
#include <QMouseEvent>

TagLabel::TagLabel(const QString &text, const QString &color, QWidget *parent)
    : QLabel(parent)
{
    setText(text);
    setObjectName("tagLabel");

    // 使用样式表设置彩色背景
    setStyleSheet(QString(
        "QLabel#tagLabel {"
        "  background-color: %1;"
        "  color: white;"
        "  padding: 2px 8px;"
        "  border-radius: 10px;"
        "  font-size: 12px;"
        "}"
    ).arg(color));

    setCursor(Qt::PointingHandCursor);
    setFixedHeight(22);
}

void TagLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(m_tagId);
    else if (event->button() == Qt::MiddleButton)
        emit removed(m_tagId);
    QLabel::mousePressEvent(event);
}

