#pragma once

#include <QObject>
#include <QPoint>
#include <QVector>

#include "netlinkwrapper.h"

struct MeasurementItem {
  QPoint pos;
  std::map<std::string, NetLink::scan_info> scan;
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
  void updateBss(std::map<std::string, NetLink::scan_info> scan);

  QVector<MeasurementItem> mItems;
  std::list<std::string> mCurrentBss;
  std::vector<std::string> mKownBssid;
};
