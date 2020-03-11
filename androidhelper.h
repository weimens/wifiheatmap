#pragma once

#include <QImage>
#include <QString>
#include <QUrl>

bool checkPermission(QString permissionstr);
QImage imageFromContentUrl(const QUrl &ImageUrl);
