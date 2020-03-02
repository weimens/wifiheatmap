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
  Q_PROPERTY(bool needsSaving READ needsSaving NOTIFY needsSavingChanged)

public:
  Document(QObject *parent = nullptr);
  void newDocument();

  Q_INVOKABLE bool save(QUrl fileUrl);
  Q_INVOKABLE void load(QUrl fileUrl);

  Measurements *measurements() const { return mMeasurements; }

  QImage mapImage() const;
  void setMapImageUrl(const QUrl &mapImageUrl);
  bool needsSaving() const;

signals:
  void mapImageChanged();
  void measurementsChanged(Measurements *measurements);
  void needsSavingChanged(bool value);

private:
  void setMapImage(const QImage &mapImage);
  void setMeasurements();
  void setNeedsSaving(bool value);

  Measurements *mMeasurements{nullptr};
  QImage mMapImage;
  bool mNeedsSaving{false};
};
