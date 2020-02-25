#include "trigger_scan.h"
#include <QCoreApplication>
#include <QSocketNotifier>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  TriggerScan s(&a);
  return a.exec();
}
