#pragma once

#include <QObject>

#include "measurementmodel.h"

#if defined(Q_OS_ANDROID)
#include "androidscan.h"
#elif defined(Q_OS_LINUX)
#include "interfacemodel.h"
#include "linuxscan.h"
#elif defined(Q_OS_WIN)
#include "windowsinterfacemodel.h"
#include "windowsscan.h"
#endif

#include <QQueue>

class Task : public QObject {
  Q_OBJECT

public:
  Task(QVector<MeasurementEntry> &results) : mResults(results){};

  void onFinished(QVector<MeasurementEntry> res) {
    for (auto r : res)
      mResults.push_back(r);
    emit done(0);
    this->disconnect();
    this->deleteLater(); // FIXME:
  }

  void onFailed(int err) {
    emit done(err);
    this->disconnect();
    this->deleteLater(); // FIXME:
  }

  virtual void run() = 0;

signals:
  void done(int err);

private:
  QVector<MeasurementEntry> &mResults;
};

#if defined(Q_OS_ANDROID)

class AndroidScanTask : public Task {
  Q_OBJECT
public:
  AndroidScanTask(AndroidScan *androidScan, QVector<MeasurementEntry> &results)
      : Task(results), mAndroidScan(androidScan) {}

  void run() {
    mAndroidScan->connect(mAndroidScan, &AndroidScan::scanFinished, this,
                          &AndroidScanTask::onFinished);
    mAndroidScan->connect(mAndroidScan, &AndroidScan::scanFailed, this,
                          &AndroidScanTask::onFailed);
    if (!mAndroidScan->measure())
      onFailed(254);
  }

private:
  AndroidScan *mAndroidScan;
};

#elif defined(Q_OS_LINUX)
class ScanTask : public Task {
  Q_OBJECT
public:
  ScanTask(LinuxScan *linuxScan, QVector<MeasurementEntry> &results)
      : Task(results), mLinuxScan(linuxScan) {}

  void run() {
    mLinuxScan->connect(mLinuxScan, &LinuxScan::scanFinished, this,
                        &ScanTask::onFinished);
    mLinuxScan->connect(mLinuxScan, &LinuxScan::scanFailed, this,
                        &ScanTask::onFailed);
    mLinuxScan->measure();
  }

private:
  LinuxScan *mLinuxScan;
};

class ConnectedTask : public Task {
  Q_OBJECT
public:
  ConnectedTask(LinuxScan *linuxScan, QVector<MeasurementEntry> &results)
      : Task(results), mLinuxScan(linuxScan) {}

  void run() {
    mLinuxScan->connect(mLinuxScan, &LinuxScan::scanFinished, this,
                        &ConnectedTask::onFinished);
    mLinuxScan->connect(mLinuxScan, &LinuxScan::scanFailed, this,
                        &ConnectedTask::onFailed);
    mLinuxScan->connected();
  }

private:
  LinuxScan *mLinuxScan;
};
#elif defined(Q_OS_WIN)
class ScanWindowsTask : public Task {
  Q_OBJECT
public:
  ScanWindowsTask(WindowsScan *windowsScan, QVector<MeasurementEntry> &results)
      : Task(results), mWindowsScan(windowsScan) {}

  void run() {
    mWindowsScan->connect(mWindowsScan, &WindowsScan::scanFinished, this,
                          &ScanWindowsTask::onFinished);
    mWindowsScan->connect(mWindowsScan, &WindowsScan::scanFailed, this,
                          &ScanWindowsTask::onFailed);
    mWindowsScan->measure();
  }

private:
  WindowsScan *mWindowsScan;
};
#endif

class TaskQueue : public QObject { // TODO: timeout?
  Q_OBJECT
public:
  TaskQueue(QObject *parent = nullptr) : QObject(parent) {}

  void enqueue(Task *task) { queue.enqueue(task); }
  int size() { return queue.size(); }

  void run(int err) {
    if (err != 0) {
      queue.clear();
      emit done(err);
    } else {
      if (!queue.isEmpty()) {
        auto task = queue.dequeue();
        task->connect(task, &Task::done, this, &TaskQueue::run);
        task->run();
      } else {
        emit done(0);
      }
    }
  }

signals:
  void done(int err);

private:
  QQueue<Task *> queue{};
};

class MeasurementController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool scan READ scan WRITE setScan NOTIFY scanChanged)

#if defined(Q_OS_ANDROID) // FIXME:
#elif defined(Q_OS_LINUX)
  Q_PROPERTY(InterfaceModel *interfaceModel READ interfaceModel NOTIFY
                 interfaceModelChanged)
  Q_PROPERTY(LinuxScan *linuxScan READ linuxScan NOTIFY linuxScanChanged)
#elif defined(Q_OS_WIN)
  Q_PROPERTY(WindowsInterfaceModel *windowsInterfaceModel READ
                 windowsInterfaceModel NOTIFY interfaceModelChanged)
#endif

public:
  explicit MeasurementController(QObject *parent = nullptr);

  Q_INVOKABLE bool measure(QPoint pos);

  bool scan();
  void setScan(bool value);

#if defined(Q_OS_ANDROID) // FIXME:
#elif defined(Q_OS_LINUX)
  InterfaceModel *interfaceModel();
  LinuxScan *linuxScan();
#elif defined(Q_OS_WINDOWS)
  WindowsInterfaceModel *windowsInterfaceModel();
#endif

signals:
  void scanChanged(bool value);
  void interfaceModelChanged();
  void linuxScanChanged();

  void scanStarted(QPoint pos);
  void scanFinished(QVector<MeasurementEntry> results);
  void scanFailed(int err);

public slots:
  void onDone(int err);

private:
  bool mScanning{false};
  QVector<MeasurementEntry> mResults{};
  bool mScan{false};
  TaskQueue mTaskqueue;
#ifdef Q_OS_ANDROID
  AndroidScan *mAndroidScan;
#elif defined(Q_OS_LINUX)
  LinuxScan *mLinuxScan;
  InterfaceModel *mInterfaceModel;
#elif defined(Q_OS_WIN)
  WindowsScan *mWindowsScan;
  WindowsInterfaceModel *mWindowsInterfaceModel;
#endif
};
