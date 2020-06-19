#include "iperf.h"

#include <QJsonObject>

#include "iperfwrapper.h"
#include "netlinkwrapper.h"

#include <fcntl.h>
#include <unistd.h>

class StdOutNull {

public:
  StdOutNull() {

    mOut = dup(STDOUT_FILENO);
    Q_ASSERT(mOut != -1); // FIXME: onerr
    mNull = open("/dev/null", O_WRONLY);
    Q_ASSERT(mNull != -1); // FIXME: onerr

    int ret = dup2(mNull, STDOUT_FILENO);
    Q_ASSERT(ret != -1); // FIXME: onerr
  }
  ~StdOutNull() {
    dup2(mOut, STDOUT_FILENO);
    close(mNull);
  }

private:
  int mOut{-1};
  int mNull{-1};
};

Iperf::Iperf(std::string host, int port, QObject *parent)
    : QObject(parent), mHost(host), mPort(port) {}

void Iperf::server_hostname(QString host) { mHost = host.toStdString(); }

void Iperf::server_port(int port) { mPort = port; }

void Iperf::run() {
  StdOutNull rd{};

  IperfTest test;
  test.role('c');
  test.server_hostname(mHost);
  test.server_port(mPort);
  test.omit(1);
  test.duration(3);
  test.reporter_interval(1);
  test.stats_interval(1);
  int ret = test.run_client();

  if (ret == -1) {
    int err = i_errno;
    emit Failed(err, QString::fromUtf8(iperf_strerror(err)));
    return;
  }
  QJsonObject root = test.json_output().object();

  double sent_bits_per_second = 0;
  double sent_retransmits = 0;
  double received_bits_per_second = 0;

  if (root.contains("end") && root["end"].isObject()) {
    QJsonObject end = root["end"].toObject();
    if (end.contains("sum_sent") && end["sum_sent"].isObject()) {
      QJsonObject sum_sent = end["sum_sent"].toObject();
      if (sum_sent.contains("bits_per_second") &&
          sum_sent["bits_per_second"].isDouble()) {
        sent_bits_per_second = sum_sent["bits_per_second"].toDouble();
      }
      if (sum_sent.contains("retransmits") &&
          sum_sent["retransmits"].isDouble()) {
        sent_retransmits = sum_sent["retransmits"].toDouble();
      }
    }
    if (end.contains("sum_received") && end["sum_received"].isObject()) {
      QJsonObject sum_received = end["sum_received"].toObject();
      if (sum_received.contains("bits_per_second") &&
          sum_received["bits_per_second"].isDouble()) {
        received_bits_per_second = sum_received["bits_per_second"].toDouble();
      }
    }
  }

  emit Finished(sent_bits_per_second, sent_retransmits,
                received_bits_per_second);
}

IPerfScan::IPerfScan(QObject *parent) {
  iperf = new Iperf{mServerHost.toStdString(), mServerPort};
  iperf->moveToThread(&workerThread);
  connect(&workerThread, &QThread::finished, iperf, &QObject::deleteLater);
  connect(this, &IPerfScan::triggerScan, iperf, &Iperf::run);
  connect(iperf, &Iperf::Finished, this, &IPerfScan::onData);
  connect(iperf, &Iperf::Failed, this, &IPerfScan::onFailed);
  connect(this, &IPerfScan::serverHostChanged, iperf, &Iperf::server_hostname);
  connect(this, &IPerfScan::serverPortChanged, iperf, &Iperf::server_port);
  workerThread.start();
}

IPerfScan::~IPerfScan() {
  workerThread.quit();
  workerThread.wait();
}

bool IPerfScan::measure(MeasurementEntry measurementEntry) {
  if (mScanning)
    return false;
  mScanning = true;
  mMeasurementEntry = measurementEntry;
  emit triggerScan();
  return true;
}

void IPerfScan::setServerHost(QString serverHost) {
  mServerHost = serverHost;
  emit serverHostChanged(serverHost);
}

void IPerfScan::setServerPort(int serverPort) {
  mServerPort = serverPort;
  emit serverPortChanged(serverPort);
}

QString IPerfScan::serverHost() { return mServerHost; }

int IPerfScan::serverPort() { return mServerPort; }

void IPerfScan::onData(double sent_bits_per_second, double sent_retransmits,
                       double received_bits_per_second) {
  if (!mMeasurementEntry.has_value())
    return;
  auto me = mMeasurementEntry.value();
  mScanning = false;
  mMeasurementEntry.reset();

  QVector<MeasurementEntry> ret{
      {MeasurementEntry{me.bss, IperfRx, received_bits_per_second},
       MeasurementEntry{me.bss, IperfTx, sent_bits_per_second},
       MeasurementEntry{me.bss, IperfRetransmits, sent_retransmits}}};

  ret.push_back(me);

  emit scanFinished(ret);
}

void IPerfScan::onFailed(int err, QString errstr) {
  mScanning = false;
  mMeasurementEntry.reset();
  // qDebug() << "Failed:" << err << errstr; FIXME:
  emit scanFailed(err, errstr);
}

void IPerfScan::setInterfaceIndex(int index) { mInterfaceIndex = index; }
