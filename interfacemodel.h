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
  Q_PROPERTY(
      int currentIndex MEMBER m_currentIndex NOTIFY currrentIndexChanged);
  Q_PROPERTY(int currentInterface MEMBER m_currentInterface NOTIFY
                 currentInterfaceChanged)

public:
  InterfaceModel(QObject *parent = nullptr)
      : QStandardItemModel(parent), m_currentIndex(0), m_currentInterface(0) {
    setItemRoleNames({{InterfaceItem::Roles::indexRole, "interface_index"},
                      {InterfaceItem::Roles::nameRole, "name"}});
    connect(this, &InterfaceModel::currrentIndexChanged, this,
            &InterfaceModel::updateCurrentInterfaceIndex);
  }

  void append(unsigned char index, QString name) {
    QStandardItem *item = new InterfaceItem(index, name);
    appendRow(QList<QStandardItem *>() << item);
  }

signals:
  void currrentIndexChanged();
  void currentInterfaceChanged(int currentInterface);

private slots:
  void updateCurrentInterfaceIndex() {
    m_currentInterface =
        data(index(m_currentIndex, 0), InterfaceItem::Roles::indexRole).toInt();
    emit currentInterfaceChanged(m_currentInterface);
  }

private:
  int m_currentIndex;
  int m_currentInterface;
};
