#pragma once

#include <QJsonDocument>
#include <iperf_api.h>
#include <string>

class IperfTest {
private:
  iperf_test *test;

public:
  IperfTest();

  void server_hostname(std::string host);
  void server_port(int port);
  void bind(std::string bind_addr);
  void omit(int omit);
  void duration(int duration);
  void reporter_interval(double reporter_interval);
  void stats_interval(double stats_interval);
  void role(char role);

  int run_client();
  QJsonDocument json_output();

  ~IperfTest();
};
