#include "bssmodel.h"

BssModel::BssModel(Document *document, QObject *parent)
    : QStandardItemModel(parent) {
  setItemRoleNames(
      {{Qt::DisplayRole, "display"}, {Qt::CheckStateRole, "checkstate"}});
  connect(this, &BssModel::dataChanged, this, &BssModel::selectionChanged);

  connect(document, &Document::measurementsChanged, this,
          &BssModel::measurementsChanged);
  measurementsChanged(document->measurements());
}

void BssModel::setHeader() {
  setHorizontalHeaderLabels(QStringList() << "ssid"
                                          << "bss"
                                          << "freq"
                                          << "ch"
                                          << "");
}

void BssModel::measurementsChanged(Measurements *measurements) {
  if (measurements) {
    connect(measurements, &Measurements::bssAdded, this, &BssModel::addBss);
    connect(this, &BssModel::selectedBssChanged, measurements,
            &Measurements::bssChanged);
  }

  clear();
  setHeader();
}

void BssModel::selectionChanged(const QModelIndex &topLeft,
                                const QModelIndex &bottomRight,
                                const QVector<int> &roles) {
  if (std::find(roles.begin(), roles.end(), Qt::CheckStateRole) !=
      roles.end()) {
    QList<QString> bss = getSelectedBss();
    emit selectedBssChanged(bss);
  }
}

void BssModel::addBss(QString bssid, QString ssid, qreal freq, qreal channel) {
  QStandardItem *item1 = new QStandardItem(ssid);
  QStandardItem *item2 = new QStandardItem(bssid);
  QStandardItem *item3 = new QStandardItem();
  item3->setData(freq, Qt::DisplayRole);
  QStandardItem *item4 = new QStandardItem();
  item4->setData(channel, Qt::DisplayRole);
  QStandardItem *item5 = new QStandardItem();
  item5->setCheckable(true);
  item5->setData(Qt::Unchecked, Qt::CheckStateRole);
  appendRow(QList<QStandardItem *>()
            << item1 << item2 << item3 << item4 << item5);
}

QList<QString> BssModel::getSelectedBss() {
  QList<QString> ret;
  for (int i = 0; i < rowCount(); ++i) {
    QStandardItem *itemCheck =
        static_cast<QStandardItem *>(itemFromIndex(index(i, 4)));
    if (itemCheck->data(Qt::CheckStateRole) == Qt::Checked) {
      QStandardItem *item =
          static_cast<QStandardItem *>(itemFromIndex(index(i, 1)));
      ret.append(item->data(Qt::DisplayRole).toString());
    }
  }
  return ret;
}
