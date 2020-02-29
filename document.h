#pragma once

#include "measurementmodel.h"
#include <QObject>
#include <QUrl>
#include <QImage>

class Document : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl mapImageUrl WRITE setMapImageUrl NOTIFY mapImageChanged)

public:
  Document(MeasurementModel *model, QObject *parent = nullptr);

  Q_INVOKABLE void save(QUrl fileUrl);
  Q_INVOKABLE void load(QUrl fileUrl);

  QImage mapImage() const;
  void setMapImageUrl(const QUrl &mapImageUrl);

signals:
    void mapImageChanged();

private:
  void setMapImage(const QImage &mapImage);

  MeasurementModel *m_model;
  QImage mMapImage;
};
