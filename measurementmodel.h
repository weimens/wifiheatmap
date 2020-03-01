#pragma once

#include <QAbstractListModel>
#include <QPoint>

#include "document.h"
#include "measurements.h"
#include "netlinkwrapper.h"
#include "trigger_scan.h"

class MeasurementModel : public QAbstractListModel {
  Q_OBJECT
  Q_ENUMS(Roles)
  Q_PROPERTY(Measurements *measurements READ measurements WRITE setMeasurements)
  Q_PROPERTY(int interfaceIndex WRITE setInterfaceIndex)

public:
  enum Roles {
    posRole,
    zRole,
    stateRole,
  };

  MeasurementModel(Document *document, TriggerScan *scanner,
                   QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

  Q_INVOKABLE bool measure(QPoint pos);

  bool updateScan(QModelIndex idx,
                  std::map<std::string, NetLink::scan_info> scan);

  Q_INVOKABLE void remove(int row);

  QList<MeasurementItem> getMeasurementItems();

  void setMeasurements(Measurements *measurements);

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  Measurements *measurements() const;

public slots:
  void setInterfaceIndex(int index);

  void scanFinished();

  void scanFailed(int err);

private:
  int mInterfaceIndex;
  TriggerScan *mScanner;
  QPersistentModelIndex mScanIndex;
  Measurements *mMeasurements;
};
