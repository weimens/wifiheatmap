#include "imageprovider.h"

ImageProvider::ImageProvider(Document *document)
    : QQuickImageProvider(QQuickImageProvider::Image), mDocument(document) {}

QImage ImageProvider::requestImage(const QString &id, QSize *size,
                                   const QSize &requestedSize) {
  QImage image;
  if (id.startsWith("mapimage"))
    image = mDocument->mapImage();
  size->setWidth(image.width());
  size->setHeight(image.height());

  return image;
}
