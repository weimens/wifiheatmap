#pragma once

#include <QObject>

class WindowsScanTrigger : public QObject {
  Q_OBJECT

public slots:
  void doScan(int interfaceIndex);

signals:
  void resultReady(const unsigned long waitResult);
};
