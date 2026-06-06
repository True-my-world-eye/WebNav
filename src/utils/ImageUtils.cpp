// ImageUtils.cpp

#include "ImageUtils.h"
#include <QDir>
#include <QPainter>
#include <QStandardPaths>

QPixmap ImageUtils::scaleImage(const QString &path, int width, int height)
{
    QPixmap pix(path);
    if (pix.isNull()) return {};

    return pix.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap ImageUtils::roundImage(const QPixmap &source)
{
    if (source.isNull()) return {};

    QPixmap result(source.size());
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(source));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(source.rect(), 4, 4);

    return result;
}

QString ImageUtils::cacheDirectory(const QString &subDir)
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                  + "/" + subDir + "/";
    QDir().mkpath(dir);
    return dir;
}

bool ImageUtils::clearCache(const QString &subDir)
{
    QDir dir(cacheDirectory(subDir));
    return dir.removeRecursively();
}
