#pragma once

#include <QList>
#include <QObject>
#include <QPoint>
#include <QProcess>
#include <QTimer>

#include "measurementmodel.h"
#include "measurements.h"

class TriggerScan : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool running MEMBER mRunning NOTIFY runningChanged)
  Q_PROPERTY(int interfaceIndex WRITE setInterfaceIndex)

public:
  TriggerScan(MeasurementModel *measurementModel, QObject *parent = nullptr);
  virtual ~TriggerScan();

  Q_INVOKABLE void start_scanner();

  Q_INVOKABLE bool measure(QPoint pos);
  QList<ScanInfo> results();
public slots:
  void onData();

  void timeout();

  void onScannerStateChanged(QProcess::ProcessState newState);

  void setInterfaceIndex(int index);

signals:
  void runningChanged();
  void scanStarted(QPoint pos);
  void scanFinished(QList<ScanInfo> results);
  void scanFailed(int err);

private:
  bool mScanning;
  QProcess *mScanner;
  int mScanNum;
  QTimer *mTimer;
  bool mRunning;
  int mInterfaceIndex{0};
};
