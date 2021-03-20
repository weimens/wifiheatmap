#pragma once

#include <QString>

class Bss {
public:
  QString bssid;
  QString ssid;
  int freq;
  int channel;

  Bss() = default;
  Bss(const Bss &) = default;
  Bss &operator=(const Bss &) = default;

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

Q_DECLARE_METATYPE(Bss);
