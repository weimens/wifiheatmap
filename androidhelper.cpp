#include "androidhelper.h"

#include <QtAndroid>

bool checkPermission(QString permissionstr) {
  auto permission = QAndroidJniObject::getStaticObjectField<jstring>(
      "android/Manifest$permission", permissionstr.toStdString().c_str());
  auto request = QtAndroid::checkPermission(permission.toString());

  if (request == QtAndroid::PermissionResult::Denied) {
    auto resultHash = QtAndroid::requestPermissionsSync(
        QStringList() << permission.toString());
    if (resultHash[permission.toString()] ==
        QtAndroid::PermissionResult::Denied) {
      return false;
    }
  }
  return true;
}
