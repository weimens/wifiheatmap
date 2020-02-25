#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>

class TriggerScan : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool running MEMBER m_running NOTIFY runningChanged)

public:
  TriggerScan(QObject *parent = nullptr)
      : QObject(parent), scanning(false), scan_num(0), m_running(false) {
    scanner = new QProcess(this);
    connect(scanner, &QProcess::readyReadStandardOutput, this,
            &TriggerScan::onData);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TriggerScan::timeout);
    connect(scanner, &QProcess::stateChanged, this,
            &TriggerScan::onScannerStateChanged);
  }

  virtual ~TriggerScan() {
    disconnect(scanner, &QProcess::readyReadStandardOutput, this,
               &TriggerScan::onData);
    if (scanner->state() == QProcess::Running) {
      scanner->write("QUIT\n");
      scanner->waitForFinished();
    }
  }

  bool trigger_scan(int devid) {
    if (scanning || scanner->state() != QProcess::Running)
      return false;
    scanning = true;
    timer->start(10000);
    scan_num++;
    scanner->write(
        QString("SCAN %1 %2\n").arg(devid).arg(scan_num).toStdString().c_str());
    return true;
  }

  Q_INVOKABLE void start_scanner() {
    if (scanner->state() == QProcess::ProcessState::Running) {
      scanner->write("QUIT\n");
      return;
    }
    QString program = "pkexec";
    QStringList arguments;
    arguments << "/usr/bin/wifiheatmap_trigger_scan";

    scanner->start(program, arguments);
    if (!scanner->waitForStarted()) {
      return;
    }
  }

public slots:
  void onData() {
    QString line = scanner->readLine();
    QStringList msg = line.split(" ");
    scanning = false;
    timer->stop();
    if (msg.length() < 1)
      return;

    if (msg[0] == "FIN" && msg.length() == 2) {
      emit scanFinished();
      return;
    } else if (msg[0] == "ERR" && msg.length() == 3) {
      int err = msg[2].toInt();
      emit scanFailed(err);
      return;
    }
    emit scanFailed(255);
  }

  void timeout() {
    scanning = false;
    timer->stop();
    emit scanFailed(254);
  }

  void onScannerStateChanged(QProcess::ProcessState newState) {
    m_running = newState == QProcess::ProcessState::Running;
    emit runningChanged();
  }

signals:
  void runningChanged();
  void scanFinished();
  void scanFailed(int err);

private:
  bool scanning;
  QProcess *scanner;
  int scan_num;
  QTimer *timer;
  bool m_running;
};
