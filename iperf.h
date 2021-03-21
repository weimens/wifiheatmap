#pragma once

#include <QObject>
#include <QPoint>
#include <QThread>
#include <optional>

#include "measurements.h"

class Iperf : public QObject {
  Q_OBJECT

private:
  std::string mHost;
  int mPort;

public:
  Iperf(std::string host, int port, QObject *parent = nullptr);

  void run();

public slots:
  void server_hostname(QString host);
  void server_port(int port);

signals:
  void Failed(int err, QString errstr);
  void Finished(double sent_bits_per_second, double sent_retransmits,
                double received_bits_per_second);
};

class IPerfScan : public QObject { // FIXME: Name scan?
  Q_OBJECT

  Q_PROPERTY(QString serverHost READ serverHost WRITE setServerHost NOTIFY
                 serverHostChanged)
  Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY
                 serverPortChanged)

public:
  explicit IPerfScan(QObject *parent);
  ~IPerfScan();

  bool measure(MeasurementEntry measurementEntry);

  void setServerHost(QString serverHost);

  void setServerPort(int serverPort);

  QString serverHost();

  int serverPort();

public slots:
  void onData(double sent_bits_per_second, double sent_retransmits,
              double received_bits_per_second);
  void onFailed(int err, QString errstr);

  void setInterfaceIndex(int index);

signals:
  void scanFinished(QVector<MeasurementEntry> results);
  void scanFailed(int err, QString message);
  void triggerScan();
  void serverHostChanged(QString serverHost);
  void serverPortChanged(int serverPort);

private:
  bool mScanning{false};
  QString mServerHost{"127.0.0.1"};
  int mServerPort{5201};
  QThread workerThread;
  Iperf *iperf;
  int mInterfaceIndex{0};
  std::optional<MeasurementEntry> mMeasurementEntry;
};
