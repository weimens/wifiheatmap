#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QProcess>
#include <QSocketNotifier>
#include <QTextStream>
#include <cerrno>
#include <iostream>
#include <system_error>

#include <net/if.h>
#include <unistd.h>

class TriggerScan : public QObject {
  Q_OBJECT

public:
  TriggerScan(QObject *parent = nullptr)
      : QObject(parent), notifier(STDIN_FILENO, QSocketNotifier::Read, this) {
    connect(&notifier, SIGNAL(activated(int)), this, SLOT(onData()));
    notifier.setEnabled(true);
  }

public slots:
  void onData() {
    QTextStream stream(stdin, QIODevice::ReadOnly);
    if (stream.status() != QTextStream::Ok || stream.atEnd()) {
      QCoreApplication::quit();
      return;
    }

    QString line = stream.readLine();
    QStringList msg = line.split(" ");
    if (msg.length() < 1)
      return;

    if (msg[0] == "QUIT") {
      QCoreApplication::quit();
      return;
    } else if (msg[0] == "SCAN" && msg.length() == 3) {
      unsigned long long devidx = msg[1].toInt();
      unsigned long long scanid = msg[2].toInt();
      if (devidx > 0 && scanid > 0) {
        errno = 0;
        char ifname[IF_NAMESIZE];
        if_indextoname(devidx, ifname);
        if (errno == 0) {
          QString interface_name(ifname);
          int err = doScan(interface_name);
          if (err == 0) {
            std::cout << "FIN " << scanid << std::endl << std::flush;
            return;
          } else {
            std::cout << "ERR " << scanid << " " << err << std::endl
                      << std::flush;
            return;
          }
        } else {
          std::cout << "ERR " << scanid << " " << errno << std::endl
                    << std::flush;
          return;
        }
      }
    }
    std::cout << "ERR" << std::endl << std::flush;
  }

protected:
  int doScan(QString interface_name) {
    QString program = "/usr/sbin/iw";
    QStringList arguments;
    arguments << "dev" << interface_name << "scan"
              << "passive";

    QProcess iw(this);
    iw.start(program, arguments);
    if (!iw.waitForStarted()) {
      return std::make_error_code(std::errc::timed_out).value();
    }

    if (!iw.waitForFinished()) {
      return std::make_error_code(std::errc::timed_out).value();
    } else {
      return iw.exitCode();
    }
  }

private:
  QSocketNotifier notifier;
};
