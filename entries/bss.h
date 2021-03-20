#pragma once

#include <QString>

struct Bss {
  QString bssid;
  QString ssid;
  int freq;
  int channel;

  Bss(QString bssid, QString ssid, int freq, int channel) { // FIXME:
    this->bssid = bssid;
    this->ssid = ssid;
    this->freq = freq;
    this->channel = channel;
  }

  bool operator==(const Bss &b) const {
    return this->bssid == b.bssid && this->ssid == b.ssid &&
           this->freq == b.freq && this->channel == b.channel;
  }
};
