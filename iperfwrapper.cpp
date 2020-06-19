#include "iperfwrapper.h"

#include <QTextStream>

IperfTest::IperfTest() {
  test = iperf_new_test();
  // if (test == nullptr) //FIXME:
  //  qDebug() << "iperf_new_test failed";
  int ret = iperf_defaults(test);
  iperf_set_test_json_output(test, 1);
}

void IperfTest::server_hostname(std::string host) { // FIXME: const_cast
  iperf_set_test_server_hostname(test, const_cast<char *>(host.c_str()));
}

void IperfTest::server_port(int port) {
  iperf_set_test_server_port(test, port);
}

void IperfTest::bind(std::string bind_addr) {
  iperf_set_test_bind_address(test, const_cast<char *>(bind_addr.c_str()));
}

void IperfTest::omit(int omit) { iperf_set_test_omit(test, omit); }

void IperfTest::duration(int duration) {
  iperf_set_test_duration(test, duration);
}

void IperfTest::reporter_interval(double reporter_interval) {
  iperf_set_test_reporter_interval(test, reporter_interval);
}

void IperfTest::stats_interval(double stats_interval) {
  iperf_set_test_stats_interval(test, stats_interval);
}

void IperfTest::role(char role) { iperf_set_test_role(test, role); }

int IperfTest::run_client() { return iperf_run_client(test); }

QJsonDocument IperfTest::json_output() {
  QTextStream output(iperf_get_test_json_output_string(test));
  return QJsonDocument::fromJson(output.readAll().toUtf8());
}

IperfTest::~IperfTest() { iperf_free_test(test); }
