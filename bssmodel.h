#pragma once

#include <QStandardItem>
#include <QStandardItemModel>

class BssModel : public QStandardItemModel {
  Q_OBJECT

public:
  BssModel(QObject *parent = nullptr);

public slots:
  void selectionChanged(const QModelIndex &topLeft,
                        const QModelIndex &bottomRight,
                        const QVector<int> &roles);

  void addBss(QString bssid, QString ssid, qreal freq, qreal channel);

signals:
  void selectedBssChanged(QList<QString>);

private:
  QList<QString> getSelectedBss();
};
