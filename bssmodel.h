#pragma once

#include "measurements.h"
#include <QStandardItem>
#include <QStandardItemModel>

class BssModel : public QStandardItemModel {
  Q_OBJECT

public:
  explicit BssModel(QObject *parent = nullptr);

public slots:
  void selectionChanged(const QModelIndex &topLeft,
                        const QModelIndex &bottomRight,
                        const QVector<int> &roles);

  void addBss(QString bssid, QString ssid, qreal freq, qreal channel);
  void measurementsChanged(Measurements *measurements);

signals:
  void selectedBssChanged(QList<QString>);

private:
  QList<QString> getSelectedBss();
  void setHeader();
};
