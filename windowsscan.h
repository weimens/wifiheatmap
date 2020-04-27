#pragma once

#include <QObject>
#include <QThread>

#include "scaninfo.h"

class WindowsScan : public QObject {
  Q_OBJECT
public:
  explicit WindowsScan(QObject *parent);
  ~WindowsScan();

  Q_INVOKABLE bool measure(QPoint pos);

  QList<ScanInfo> results();
  void onData(unsigned long waitResult);

public slots:
  void setInterfaceIndex(int index);

signals:
  void scanStarted(QPoint pos);
  void scanFinished(QList<ScanInfo> results);
  void scanFailed(int err);
  void triggerScan(int interfaceIndex);

private:
  bool mScanning{false};
  int mInterfaceIndex{0};
  QThread workerThread;
};
