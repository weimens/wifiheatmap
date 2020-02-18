#pragma once

#include <QAbstractListModel>
#include <QPoint>

#include <QStandardItem>
#include <QStandardItemModel>

class InterfaceItem : public QStandardItem {
public:
  enum Roles {
    indexRole = Qt::UserRole + 6,
    nameRole = Qt::UserRole + 7,
  };

  InterfaceItem(unsigned char index, QString name) {
    QStandardItem();
    setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    setData(index, InterfaceItem::Roles::indexRole);
    setData(name, InterfaceItem::Roles::nameRole);
  }

  int type() const override { return QStandardItem::UserType + 2; }
};

class InterfaceModel : public QStandardItemModel {
  Q_OBJECT

public:
  InterfaceModel() {
    QStandardItemModel(1, 0);
    setItemRoleNames({{InterfaceItem::Roles::indexRole, "interface_index"},
                      {InterfaceItem::Roles::nameRole, "name"}});
  }

  void append(unsigned char index, QString name) {
    QStandardItem *item = new InterfaceItem(index, name);
    appendRow(QList<QStandardItem *>() << item);
  }
};
