#pragma once

#include <QList>
#include <QObject>
#include <QPoint>
#include <QProcess>
#include <QTimer>

#include "entries.h"

class LinuxScan : public QObject {
  Q_OBJECT
  Q_PROPERTY(int interfaceIndex WRITE setInterfaceIndex)

public:
  explicit LinuxScan(QObject *parent = nullptr);
  virtual ~LinuxScan();

  void start_scanner();
  void stop_scanner();

  std::optional<MeasurementEntry> connected();
  bool measure();
  QVector<MeasurementEntry> results();
public slots:
  void onData();

  void timeout();

  void onScannerStateChanged(QProcess::ProcessState newState);

  void setInterfaceIndex(int index);

signals:
  void stopped(bool value);
  void scanFinished(QVector<MeasurementEntry> results);
  void scanFailed(int err, QString message);

private:
  bool mScanning;
  QProcess *mScanner;
  int mScanNum;
  QTimer *mTimer;
  int mInterfaceIndex{0};
};
