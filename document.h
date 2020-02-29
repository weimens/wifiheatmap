#pragma once

#include "measurementmodel.h"
#include <QObject>
#include <QUrl>
#include <QImage>

class Document : public QObject {
  Q_OBJECT
  Q_PROPERTY(QImage mapImage READ mapImage WRITE setMapImage NOTIFY mapImageChanged)

public:
  Document(MeasurementModel *model, QObject *parent = nullptr);

  Q_INVOKABLE void save(QUrl fileUrl);
  Q_INVOKABLE void load(QUrl fileUrl);

  QImage mapImage() const;
  void setMapImage(const QImage &mapImage);
  void setMapImageUrl(const QUrl &mapImageUrl);

signals:
    void mapImageChanged();

private:
  MeasurementModel *m_model;
  QImage mMapImage;
};
