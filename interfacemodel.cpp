#include "interfacemodel.h"

InterfaceItem::InterfaceItem(unsigned char index, QString name) {
  QStandardItem();
  setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  setData(index, InterfaceItem::Roles::indexRole);
  setData(name, InterfaceItem::Roles::nameRole);
}

int InterfaceItem::type() const { return QStandardItem::UserType + 2; }

InterfaceModel::InterfaceModel(QObject *parent)
    : QStandardItemModel(parent), mCurrentIndex(0), mCurrentInterface(0) {
  setItemRoleNames({{InterfaceItem::Roles::indexRole, "interface_index"},
                    {InterfaceItem::Roles::nameRole, "name"}});
  connect(this, &InterfaceModel::currrentIndexChanged, this,
          &InterfaceModel::updateCurrentInterfaceIndex);
}

void InterfaceModel::append(unsigned char index, QString name) {
  QStandardItem *item = new InterfaceItem(index, name);
  appendRow(QList<QStandardItem *>() << item);
}

void InterfaceModel::updateCurrentInterfaceIndex() {
  mCurrentInterface =
      data(index(mCurrentIndex, 0), InterfaceItem::Roles::indexRole).toInt();
  emit currentInterfaceChanged(mCurrentInterface);
}
