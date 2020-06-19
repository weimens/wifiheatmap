#include "measurementcontroller.h"

MeasurementController::MeasurementController(StatusQueue *statusQueue,
                                             QObject *parent)
    : QObject(parent), mStatusQueue(statusQueue) {

#ifdef Q_OS_ANDROID
  mAndroidScan = new AndroidScan(this);
#elif defined(Q_OS_LINUX)
  mLinuxScan = new LinuxScan(this);
  QObject::connect(mLinuxScan, &LinuxScan::stopped, this,
                   &MeasurementController::setScan);
  emit linuxScanChanged();

  mIPerfScan = new IPerfScan(this);
  emit iperfScanChanged();

  mInterfaceModel = new InterfaceModel(this);
  QObject::connect(mInterfaceModel, &InterfaceModel::currentInterfaceChanged,
                   mLinuxScan, &LinuxScan::setInterfaceIndex);
  QObject::connect(mInterfaceModel, &InterfaceModel::currentInterfaceChanged,
                   mIPerfScan, &IPerfScan::setInterfaceIndex);
  emit interfaceModelChanged();

#elif defined(Q_OS_WIN)
  mWindowsScan = new WindowsScan(this);
  mWindowsInterfaceModel = new WindowsInterfaceModel(this);
  QObject::connect(mWindowsInterfaceModel,
                   &WindowsInterfaceModel::currentInterfaceChanged,
                   mWindowsScan, &WindowsScan::setInterfaceIndex);
  emit interfaceModelChanged();
#endif

  mTaskqueue.connect(&mTaskqueue, &TaskQueue::done, this,
                     &MeasurementController::onDone);
}

bool MeasurementController::measure(QPoint pos) {
  if (mScanning)
    return false;
  mScanning = true;
#ifdef Q_OS_ANDROID
  AndroidScanTask *t = new AndroidScanTask{mAndroidScan, mResults};
  mTaskqueue.enqueue(t);
#elif defined(Q_OS_LINUX)
  if (mScan) {
    ScanTask *t = new ScanTask{mLinuxScan, mResults};
    mTaskqueue.enqueue(t);
  }
  if (mIperf) {
    Task *t = new IperfTask{mIPerfScan, mLinuxScan, mResults};
    mTaskqueue.enqueue(t);
  }
  if (!mScan) {
    Task *t = new ConnectedTask{mLinuxScan, mResults};
    mTaskqueue.enqueue(t);
  }
#elif defined(Q_OS_WIN)
  ScanWindowsTask *t = new ScanWindowsTask{mWindowsScan, mResults};
  mTaskqueue.enqueue(t);
#endif
  if (mTaskqueue.size() > 0) {
    emit scanStarted(pos);
    mTaskqueue.run(0, "");
    return true;
  }
  return false;
}

bool MeasurementController::iperf() { return mIperf; }

void MeasurementController::setIperf(bool value) {
  if (mIperf == value)
    return;
  mIperf = value;
  emit iperfChanged(mIperf);
}

bool MeasurementController::scan() { return mScan; }

void MeasurementController::setScan(bool value) {
  if (mScan == value)
    return;

#if defined(Q_OS_ANDROID) // FIXME:
#elif defined(Q_OS_LINUX)
  if (value) {
    mLinuxScan->start_scanner();
  } else {
    mLinuxScan->blockSignals(true);
    mLinuxScan->stop_scanner();
    mLinuxScan->blockSignals(false);
  }
#endif

  mScan = value;
  emit scanChanged(mScan);
}

#if defined(Q_OS_ANDROID) // FIXME:
#elif defined(Q_OS_LINUX)
IPerfScan *MeasurementController::iperfScan() { return mIPerfScan; }

InterfaceModel *MeasurementController::interfaceModel() {
  return mInterfaceModel;
}

LinuxScan *MeasurementController::linuxScan() { return mLinuxScan; }
#elif defined(Q_OS_WIN)
WindowsInterfaceModel *MeasurementController::windowsInterfaceModel() {
  return mWindowsInterfaceModel;
}
#endif

void MeasurementController::onDone(int err, QString message) {
  if (err == 0) {
    emit scanFinished(mResults);
  } else {
    mStatusQueue->push(message);
    emit scanFailed(0);
  }
  mScanning = false;
  mResults = {};
}
