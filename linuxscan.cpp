#include "linuxscan.h"
#include "netlinkwrapper.h"

LinuxScan::LinuxScan(QObject *parent)
    : QObject(parent), mScanning(false), mScanNum(0), mRunning(false) {

  mScanner = new QProcess(this);
  connect(mScanner, &QProcess::readyReadStandardOutput, this,
          &LinuxScan::onData);
  mTimer = new QTimer(this);
  connect(mTimer, &QTimer::timeout, this, &LinuxScan::timeout);
  connect(mScanner, &QProcess::stateChanged, this,
          &LinuxScan::onScannerStateChanged);
}

LinuxScan::~LinuxScan() {
  disconnect(mScanner, &QProcess::readyReadStandardOutput, this,
             &LinuxScan::onData);
  if (mScanner->state() == QProcess::Running) {
    mScanner->write("QUIT\n");
    mScanner->waitForFinished();
  }
}

bool LinuxScan::measure(QPoint pos) {
  if (mScanning || mScanner->state() != QProcess::Running)
    return false;
  mScanning = true;
  mTimer->start(10000);
  mScanNum++;
  mScanner->write(QString("SCAN %1 %2\n")
                      .arg(mInterfaceIndex)
                      .arg(mScanNum)
                      .toStdString()
                      .c_str());
  emit scanStarted(pos);
  return true;
}

void LinuxScan::start_scanner() {
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

QList<ScanInfo> LinuxScan::results() {
  NetLink::Nl80211 nl80211;
  NetLink::MessageScan msg(mInterfaceIndex);
  nl80211.sendMessageWait(&msg);
  const std::map<std::string, NetLink::scan_info> &scans = msg.getScan();
  auto scanInfos = QList<ScanInfo>{};
  for (auto s : scans) {
    auto scan = ScanInfo{QString::fromStdString(s.second.bssid),
                         QString::fromStdString(s.second.ssid),
                         s.second.last_seen,
                         s.second.freq,
                         s.second.signal,
                         s.second.channel};
    scanInfos.push_back(scan);
  }
  return scanInfos;
}

void LinuxScan::onData() {
  QString line = mScanner->readLine();
  QStringList msg = line.split(" ");
  mScanning = false;
  mTimer->stop();
  if (msg.length() < 1)
    return;

  if (msg[0] == "FIN" && msg.length() == 2) {
    auto res = results();
    if (res.size() > 0) {
      emit scanFinished(res);
    } else {
      emit scanFailed(254);
    }

    return;
  } else if (msg[0] == "ERR" && msg.length() == 3) {
    int err = msg[2].toInt();
    emit scanFailed(err);
    return;
  }
  emit scanFailed(255);
}

void LinuxScan::timeout() {
  mScanning = false;
  mTimer->stop();
  emit scanFailed(254);
}

void LinuxScan::onScannerStateChanged(QProcess::ProcessState newState) {
  mRunning = newState == QProcess::ProcessState::Running;
  emit runningChanged();
}

void LinuxScan::setInterfaceIndex(int index) { mInterfaceIndex = index; }
