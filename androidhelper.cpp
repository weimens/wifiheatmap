#include "androidhelper.h"

#include <QFile>
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

QImage imageFromContentUrl(const QUrl &ImageUrl) {
  auto jmode = QAndroidJniObject::fromString(QLatin1String("r"));
  auto jpath =
      QAndroidJniObject::fromString(QLatin1String(ImageUrl.url().toLatin1()));
  auto uri = QAndroidJniObject::callStaticObjectMethod(
      "android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;",
      jpath.object<jstring>());

  auto contentResolver = QtAndroid::androidActivity().callObjectMethod(
      "getContentResolver", "()Landroid/content/ContentResolver;");
  auto parcelFileDescriptor = contentResolver.callObjectMethod(
      "openFileDescriptor",
      "(Landroid/net/Uri;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
      uri.object<jobject>(), jmode.object<jobject>());

  QFile file;
  file.open((parcelFileDescriptor.callMethod<jint>("getFd", "()I")),
            QIODevice::ReadOnly);
  QImage image;
  image.loadFromData(file.readAll());
  parcelFileDescriptor.callMethod<void>("close", "()V");
  return image;
}
