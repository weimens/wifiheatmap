#pragma once

#include "measurements.h"
#include <QImage>
#include <QObject>
#include <QUndoStack>
#include <QUrl>
#include <memory>

class Document : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      Measurements *measurements READ measurements NOTIFY measurementsChanged)
  Q_PROPERTY(QUrl mapImageUrl WRITE setMapImageUrl NOTIFY mapImageChanged)
  Q_PROPERTY(bool needsSaving READ needsSaving NOTIFY needsSavingChanged)

public:
  Document(QUndoStack *undoStack, QObject *parent = nullptr);
  Q_INVOKABLE void newDocument();

  Q_INVOKABLE bool save(QUrl fileUrl);
  Q_INVOKABLE void load(QUrl fileUrl);

  Measurements *measurements() const;

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

  std::unique_ptr<Measurements> mMeasurements{nullptr};
  QUndoStack *mUndoStack;
  QImage mMapImage;
  bool mNeedsSaving{false};
  void read(QByteArray data);
};
