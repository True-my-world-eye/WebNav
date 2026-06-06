// ColorUtils.cpp

#include "ColorUtils.h"
#include <QCryptographicHash>

QColor ColorUtils::generateColor(const QString &seed)
{
    QByteArray hash = QCryptographicHash::hash(seed.toUtf8(), QCryptographicHash::Md5);
    // 取前三个字节作为 RGB，确保饱和度适中
    int r = 100 + (static_cast<unsigned char>(hash[0]) % 156);
    int g = 100 + (static_cast<unsigned char>(hash[1]) % 156);
    int b = 100 + (static_cast<unsigned char>(hash[2]) % 156);
    return QColor(r, g, b);
}

QString ColorUtils::toHex(const QColor &color)
{
    return color.name();
}

QColor ColorUtils::textColorForBackground(const QColor &bg)
{
    // 标准亮度公式判断
    int luminance = 0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue();
    return (luminance > 128) ? QColor(Qt::black) : QColor(Qt::white);
}

QVector<QString> ColorUtils::presetTagColors()
{
    return {
        "#5B9BD5",  // 蓝色
        "#70AD47",  // 绿色
        "#FFC000",  // 橙色
        "#ED7D31",  // 深橙
        "#BF4B4B",  // 红色
        "#9B59B6",  // 紫色
        "#1ABC9C",  // 青色
        "#E91E63",  // 粉色
        "#607D8B",  // 灰蓝
        "#795548",  // 棕色
    };
}
