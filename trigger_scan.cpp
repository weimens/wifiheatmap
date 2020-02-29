#include "trigger_scan.h"

TriggerScan::TriggerScan(QObject *parent)
    : QObject(parent), mScanning(false), mScanNum(0), mRunning(false) {
  mScanner = new QProcess(this);
  connect(mScanner, &QProcess::readyReadStandardOutput, this,
          &TriggerScan::onData);
  mTimer = new QTimer(this);
  connect(mTimer, &QTimer::timeout, this, &TriggerScan::timeout);
  connect(mScanner, &QProcess::stateChanged, this,
          &TriggerScan::onScannerStateChanged);
}

TriggerScan::~TriggerScan() {
  disconnect(mScanner, &QProcess::readyReadStandardOutput, this,
             &TriggerScan::onData);
  if (mScanner->state() == QProcess::Running) {
    mScanner->write("QUIT\n");
    mScanner->waitForFinished();
  }
}

bool TriggerScan::trigger_scan(int devid) {
  if (mScanning || mScanner->state() != QProcess::Running)
    return false;
  mScanning = true;
  mTimer->start(10000);
  mScanNum++;
  mScanner->write(
      QString("SCAN %1 %2\n").arg(devid).arg(mScanNum).toStdString().c_str());
  return true;
}

void TriggerScan::start_scanner() {
  if (mScanner->state() == QProcess::ProcessState::Running) {
    mScanner->write("QUIT\n");
    return;
  }
  QString program = "pkexec";
  QStringList arguments;
  arguments << "/usr/bin/wifiheatmap_trigger_scan";

  mScanner->start(program, arguments);
  if (!mScanner->waitForStarted()) {
    return;
  }
}

void TriggerScan::onData() {
  QString line = mScanner->readLine();
  QStringList msg = line.split(" ");
  mScanning = false;
  mTimer->stop();
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

void TriggerScan::timeout() {
  mScanning = false;
  mTimer->stop();
  emit scanFailed(254);
}

void TriggerScan::onScannerStateChanged(QProcess::ProcessState newState) {
  mRunning = newState == QProcess::ProcessState::Running;
  emit runningChanged();
}
