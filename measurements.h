#pragma once

#include <QObject>
#include <QPoint>
#include <QVector>

struct ScanInfo {
  QString bssid;
  QString ssid;
  int lastSeen;
  int freq;
  float signal;
  int channel;
};

struct MeasurementItem {
  QPoint pos;
  std::map<QString, ScanInfo> scan;
};

class Measurements : public QObject {
  Q_OBJECT

public:
  explicit Measurements(QObject *parent = nullptr);

  QVector<MeasurementItem> items() const;

  bool setItemAt(int index, const MeasurementItem &item);
  qreal maxZAt(int index) const;
  QPoint posAt(int index) const;

signals:
  void preItemAppended();
  void postItemAppended();

  void ItemChanged();

  void preItemRemoved(int index);
  void postItemRemoved();

  void heatMapChanged();
  void bssAdded(QString bssid, QString ssid, qreal freq, qreal channel);

public slots:
  void appendItem(MeasurementItem item);
  void removeAt(int index);

  void bssChanged(QList<QString> bss);

private:
  void updateBss(std::map<QString, ScanInfo> scan);

  QVector<MeasurementItem> mItems;
  std::list<QString> mCurrentBss;
  std::vector<QString> mKownBssid;
};
