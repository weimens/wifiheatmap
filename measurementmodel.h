#pragma once

#include <QAbstractListModel>
#include <QPoint>
#include <cmath>

#include "netlinkwrapper.h"
#include "trigger_scan.h"

struct MeasurementItem {
  QPoint pos;
  std::map<std::string, NetLink::scan_info> scan;
};

class MeasurementModel : public QAbstractListModel {
  Q_OBJECT
  Q_ENUMS(Roles)

public:
  enum Roles {
    posRole,
    zRole,
    stateRole,
  };

  MeasurementModel(TriggerScan *scanner, QObject *parent = nullptr);

  void setCurrentBSS(std::list<std::string> bss);

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

  bool insertRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override;

  Q_INVOKABLE void measure(QPoint pos);

  void append(MeasurementItem mi);

  void updateScan(QModelIndex idx,
                  std::map<std::string, NetLink::scan_info> scan);

  bool removeRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override;

  Q_INVOKABLE void remove(int row, int count = 1);

  QList<MeasurementItem> getMeasurementItems();

public slots:
  void setInterfaceIndex(int index);

  void bssChanged(QList<QString> bss);

  void scanFinished();

  void scanFailed(int err);

signals:
  void heatMapChanged();
  void bssAdded(QString bssid, QString ssid, qreal freq, qreal channel);

private:
  float getMaxZ(const MeasurementItem &item) const;

  int interface_index;
  TriggerScan *m_scanner;
  std::list<std::string> current_bss;
  std::vector<std::string> kown_bssid;
  QList<MeasurementItem> m_list;
  QPersistentModelIndex m_scan_index;
};
