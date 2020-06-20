#pragma once

#include <QObject>
#include <QPoint>

#include "entries.h"

class AndroidScan : public QObject {
  Q_OBJECT
public:
  explicit AndroidScan(QObject *parent);

  std::optional<MeasurementEntry> connected();
  Q_INVOKABLE bool measure();

  QVector<MeasurementEntry> results();
  Q_INVOKABLE void onData();
signals:
  void scanFinished(QVector<MeasurementEntry> results);
  void scanFailed(int err, QString message);

private:
  void registerNativeMethods();
  bool mScanning{false};
};
