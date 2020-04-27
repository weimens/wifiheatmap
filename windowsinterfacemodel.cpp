#include "windowsinterfacemodel.h"
#include "wlanapiwrapper.h"

WindowsInterfaceItem::WindowsInterfaceItem(unsigned char index, QString name) {
  QStandardItem();
  setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  setData(index, WindowsInterfaceItem::Roles::indexRole);
  setData(name, WindowsInterfaceItem::Roles::nameRole);
}

int WindowsInterfaceItem::type() const { return QStandardItem::UserType + 2; }

WindowsInterfaceModel::WindowsInterfaceModel(QObject *parent)
    : QStandardItemModel(parent), mCurrentIndex(0), mCurrentInterface(0) {
  setItemRoleNames({{WindowsInterfaceItem::Roles::indexRole, "interface_index"},
                    {WindowsInterfaceItem::Roles::nameRole, "name"}});
  connect(this, &WindowsInterfaceModel::currrentIndexChanged, this,
          &WindowsInterfaceModel::updateCurrentInterfaceIndex);
  loadInterfaces();
}

void WindowsInterfaceModel::loadInterfaces(){
    WLANAPI::Handle handle;
    WLANAPI::Interfaces interfaces(handle);

    for(unsigned long i = 0; i < interfaces.size(); ++i){
        this->append(i, QString::fromWCharArray(interfaces.descriptionByIndex(i).c_str()));
    }
}

void WindowsInterfaceModel::append(unsigned char index, QString name) {
  QStandardItem *item = new WindowsInterfaceItem(index, name);
  appendRow(QList<QStandardItem *>() << item);
}

void WindowsInterfaceModel::updateCurrentInterfaceIndex() {
  mCurrentInterface =
      data(index(mCurrentIndex, 0), WindowsInterfaceItem::Roles::indexRole).toInt();
  emit currentInterfaceChanged(mCurrentInterface);
}
