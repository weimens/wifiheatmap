#pragma once

#include <QStandardItem>
#include <QStandardItemModel>

class bssItem : public QStandardItem {
public:
  enum Roles {
    bssidRole = Qt::UserRole + 1,
    ssidRole = Qt::UserRole + 2,
    freqRole = Qt::UserRole + 3,
    channelRole = Qt::UserRole + 4,
    selectedRole = Qt::UserRole + 5
  };

  bssItem(QString ssid, QString bss, qreal freq, int channel,
          bool selected = false) {
    QStandardItem();
    setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    setData(ssid, bssItem::Roles::ssidRole);
    setData(bss, bssItem::Roles::bssidRole);
    setData(freq, bssItem::Roles::freqRole);
    setData(channel, bssItem::Roles::channelRole);
    setData(selected, bssItem::Roles::selectedRole);
  }

  int type() const override { return QStandardItem::UserType + 1; }
};

class BssModel : public QStandardItemModel {
  Q_OBJECT

public:
  BssModel() {
    QStandardItemModel(1, 0);
    setItemRoleNames({{bssItem::Roles::ssidRole, "ssid"},
                      {bssItem::Roles::bssidRole, "bss"},
                      {bssItem::Roles::freqRole, "freq"},
                      {bssItem::Roles::channelRole, "channel"},
                      {bssItem::Roles::selectedRole, "selected"}});
    QObject::connect(this, &BssModel::dataChanged, this,
                     &BssModel::selectionChanged);
  }

public slots:
  void selectionChanged(const QModelIndex &topLeft,
                        const QModelIndex &bottomRight,
                        const QVector<int> &roles) {
    if (std::find(roles.begin(), roles.end(), bssItem::Roles::selectedRole) !=
        roles.end()) {
      QList<QString> bss = getSelectedBss();
      emit selectedBssChanged(bss);
    }
  }

  void addBss(QString bssid, QString ssid, qreal freq, qreal channel) {
    QStandardItem *item = new bssItem(ssid, bssid, freq, channel);
    appendRow(QList<QStandardItem *>() << item);
  }

signals:
  void selectedBssChanged(QList<QString>);

private:
  QList<QString> getSelectedBss() {
    QList<QString> ret;
    for (int i = 0; i < rowCount(); ++i) {
      bssItem *item = static_cast<bssItem *>(itemFromIndex(index(i, 0)));
      if (item->data(bssItem::Roles::selectedRole).toBool())
        ret.append(item->data(bssItem::Roles::bssidRole).toString());
    }
    return ret;
  }
};
