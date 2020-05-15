#include "windowsscan.h"
#include "windowsscantrigger.h"
#include "wlanapiwrapper.h"

#include <QPoint>

WindowsScan::WindowsScan(QObject *parent) : QObject(parent) {
  WindowsScanTrigger *worker = new WindowsScanTrigger;
  worker->moveToThread(&workerThread);
  connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
  connect(this, &WindowsScan::triggerScan, worker, &WindowsScanTrigger::doScan);
  connect(worker, &WindowsScanTrigger::resultReady, this, &WindowsScan::onData);
  workerThread.start();
}

WindowsScan::~WindowsScan() {
  workerThread.quit();
  workerThread.wait();
}

QVector<MeasurementEntry> WindowsScan::results() {
  auto scanInfos = QVector<MeasurementEntry>{};

  WLANAPI::Handle handle;
  WLANAPI::BssList bssList(handle, mInterfaceIndex);
  for (auto scanInfo : bssList.getScan()) {
    auto scan = MeasurementEntry{
        Bss{QString::fromStdString(scanInfo.bssid),
            QString::fromStdString(scanInfo.ssid), scanInfo.freq / 1000, 0},
        WiFiSignal, static_cast<float>(scanInfo.signal)};
    scanInfos.push_back(scan);
  }

  return scanInfos;
}

void WindowsScan::onData(unsigned long waitResult) {
  mScanning = false;

  if (waitResult != 0) {
    emit scanFailed(waitResult);
    return;
  }

  auto res = results();
  if (res.size() > 0) {
    emit scanFinished(res);
  } else {
    emit scanFailed(254);
  }
}

bool WindowsScan::measure(QPoint pos) {
  if (mScanning)
    return false;
  mScanning = true;

  emit scanStarted(pos);
  emit triggerScan(mInterfaceIndex);

  return false;
}

void WindowsScan::setInterfaceIndex(int index) { mInterfaceIndex = index; }
