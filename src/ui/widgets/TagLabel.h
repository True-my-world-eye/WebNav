// TagLabel.h — 彩色标签气泡控件
// 显示一个带颜色的标签文本，支持点击和删除

#pragma once
#include <QLabel>

class TagLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TagLabel(const QString &text, const QString &color = "#5B9BD5",
                      QWidget *parent = nullptr);

signals:
    void clicked(int tagId);
    void removed(int tagId);

    // 设置标签 ID
    void setTagId(int id) { m_tagId = id; }
    int tagId() const { return m_tagId; }

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_tagId = -1;
};
