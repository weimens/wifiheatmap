#pragma once

#include <QString>

struct ScanInfo {
  QString bssid;
  QString ssid;
  int lastSeen;
  int freq;
  float signal;
  int channel;
};
