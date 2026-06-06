// ImageUtils.h — 图片工具函数

#pragma once
#include <QString>
#include <QPixmap>

class ImageUtils
{
public:
    // 缩放图片到指定尺寸
    static QPixmap scaleImage(const QString &path, int width, int height);

    // 生成圆形裁剪图（用于 favicon 显示）
    static QPixmap roundImage(const QPixmap &source);

    // 获取缓存目录
    static QString cacheDirectory(const QString &subDir);

    // 清理无效缓存文件
    static bool clearCache(const QString &subDir);
};
