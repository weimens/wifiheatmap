#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>

class TriggerScan : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool running MEMBER mRunning NOTIFY runningChanged)

public:
  TriggerScan(QObject *parent = nullptr);

  virtual ~TriggerScan();

  bool trigger_scan(int devid);

  Q_INVOKABLE void start_scanner();

public slots:
  void onData();

  void timeout();

  void onScannerStateChanged(QProcess::ProcessState newState);

signals:
  void runningChanged();
  void scanFinished();
  void scanFailed(int err);

private:
  bool mScanning;
  QProcess *mScanner;
  int mScanNum;
  QTimer *mTimer;
  bool mRunning;
};
