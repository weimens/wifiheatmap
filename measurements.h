#pragma once

#include <QObject>
#include <QPoint>
#include <QVector>

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

struct Measurement {
  Position pos;
  Bss bss;
  double value;

  bool operator==(const Measurement &b) const {
    return this->bss == b.bss && this->pos == b.pos;
  }
};

class Measurements : public QObject {
  Q_OBJECT

public:
  explicit Measurements(QObject *parent = nullptr);

  const QVector<Measurement> measurements() const;
  const QVector<Position> positions() const;
  const QVector<Bss> bsss() const;

  qreal maxZAt(int index) const;
  int countAt(int index) const;
  QVector<Measurement> measurementsAt(Position position) const;

  void newMeasurementsAtPosition(Position position,
                                 const QVector<QPair<Bss, double>> &values);

  void addPosition(Position position);
  QVector<Measurement> removePosition(Position position);
  void updatePosition(Position oldPosition, Position newPosition);

signals:
  void preMeasurementAppended(int index, int count = 1);
  void postMeasurementAppended();
  void prePositionAppended(int index, int count = 1);
  void postPositionAppended();
  void preBssAppended(int index, int count = 1);
  void postBssAppended();

  void preMeasurementRemoved(int index, int count = 1);
  void postMeasurementRemoved();
  void prePositionRemoved(int index, int count = 1);
  void postPositionRemoved();
  void preBssRemoved(int index, int count = 1);
  void postBssRemoved();

  void positionChanged(int index);

  void heatMapChanged();

public slots:
  void selectedBssChanged(QVector<Bss> selectedBss);

private:
  QVector<Measurement> mMeasurements;
  QVector<Position> mPositions;
  QVector<Bss> mBss;

  QVector<Bss> mCurrentBss;
};
