#pragma once

#include <QPoint>
#include <QString>

struct Position {
  QPoint pos;

public:
  bool operator==(const Position &b) const { return this->pos == b.pos; }
};

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

enum MeasurementType { WiFiSignal, IperfRx, IperfTx, IperfRetransmits };

struct MeasurementEntry {
  Bss bss;
  double value;
  MeasurementType measurementType;

  MeasurementEntry(Bss bss, MeasurementType measurementType, double value)
      : bss(bss), measurementType(measurementType), value(value) {}
};

struct Measurement {
  Position pos;
  Bss bss;
  MeasurementType measurementType;
  double value;

  bool operator==(const Measurement &b) const {
    return this->bss == b.bss && this->pos == b.pos &&
           this->measurementType == b.measurementType;
  }
};
