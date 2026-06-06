// ColorUtils.h — 颜色工具函数

#pragma once
#include <QString>
#include <QColor>

class ColorUtils
{
public:
    // 从字符串生成稳定的随机颜色（用于自动分配标签颜色）
    static QColor generateColor(const QString &seed);

    // 获取颜色的十六进制字符串
    static QString toHex(const QColor &color);

    // 根据背景色判断应使用黑色还是白色文字
    static QColor textColorForBackground(const QColor &bg);

    // 预设标签颜色列表
    static QVector<QString> presetTagColors();
};
