#pragma once

#include "measurements.h"
#include <QImage>
#include <QObject>
#include <QUrl>

class Document : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      Measurements *measurements READ measurements NOTIFY measurementsChanged)
  Q_PROPERTY(QUrl mapImageUrl WRITE setMapImageUrl NOTIFY mapImageChanged)

public:
  Document(QObject *parent = nullptr);
  void newDocument();

  Q_INVOKABLE void save(QUrl fileUrl);
  Q_INVOKABLE void load(QUrl fileUrl);

  Measurements *measurements() const { return mMeasurements; }

  QImage mapImage() const;
  void setMapImageUrl(const QUrl &mapImageUrl);

signals:
  void mapImageChanged();
  void measurementsChanged(Measurements *measurements);

private:
  void setMapImage(const QImage &mapImage);
  void setMeasurements();

  Measurements *mMeasurements{nullptr};
  QImage mMapImage;
};
