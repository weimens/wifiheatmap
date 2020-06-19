#pragma once

#include <QObject>
#include <QThread>
#include <QVector>

#include "entries.h"

class WindowsScan : public QObject {
  Q_OBJECT
public:
  explicit WindowsScan(QObject *parent);
  ~WindowsScan();

  bool measure();

  QVector<MeasurementEntry> results();
  void onData(unsigned long waitResult);

public slots:
  void setInterfaceIndex(int index);

signals:
  void scanFinished(QVector<MeasurementEntry> results);
  void scanFailed(int err);
  void triggerScan(int interfaceIndex);

private:
  bool mScanning{false};
  int mInterfaceIndex{0};
  QThread workerThread;
};
