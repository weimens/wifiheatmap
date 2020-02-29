#pragma once

#include <QImage>
#include <QQuickImageProvider>

#include "document.h"

class ImageProvider : public QQuickImageProvider {
public:
  ImageProvider(Document *document);

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override;

private:
  Document *mDocument;
};
