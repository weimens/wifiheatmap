#pragma once

#include <QObject>

#include "measurementmodel.h"
#include "measurements.h"

class AndroidScan : public QObject {
  Q_OBJECT
public:
  AndroidScan(MeasurementModel *measurementModel, QObject *parent);

  Q_INVOKABLE bool measure(QPoint pos);

  QList<ScanInfo> results();
  Q_INVOKABLE void onData();
signals:
  void scanStarted(QPoint pos);
  void scanFinished(QList<ScanInfo> results);
  void scanFailed(int err);

private:
  void registerNativeMethods();
  bool mScanning{false};
};
