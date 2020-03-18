#include "androidscan.h"
#include "androidhelper.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#include <functional>

AndroidScan::AndroidScan(QObject *parent)
    : QObject(parent) {
  registerNativeMethods();
}

static std::function<void(JNIEnv *env, jobject thiz)> fromJavaScanFinished;
void call_fromJavaScanFinished(JNIEnv *env, jobject thiz) {
  fromJavaScanFinished(env, thiz);
}

void AndroidScan::registerNativeMethods() {
  fromJavaScanFinished = [this](JNIEnv *env, jobject thiz) {
    QMetaObject::invokeMethod(this, "onData", Qt::QueuedConnection);
  };

  JNINativeMethod methods[]{
      {"scanFinished", "()V",
       reinterpret_cast<void *>(call_fromJavaScanFinished)}};

  QAndroidJniObject javaClass(
      "org/github/weimens/wifiheatmap/WifiScanReceiver");
  QAndroidJniEnvironment env;
  auto objectClass = env->GetObjectClass(javaClass.object<jobject>());
  env->RegisterNatives(objectClass, methods,
                       sizeof(methods) / sizeof(methods[0]));
  env->DeleteLocalRef(objectClass);
}

QList<ScanInfo> AndroidScan::results() {
  auto scanInfos = QList<ScanInfo>{};

  if (!checkPermission("ACCESS_FINE_LOCATION")) {
    return scanInfos;
  }

  auto service_name = QAndroidJniObject::getStaticObjectField<jstring>(
      "android/content/Context", "WIFI_SERVICE");
  if (service_name.isValid()) {
    auto wifiManager = QtAndroid::androidActivity().callObjectMethod(
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
        service_name.object<jstring>());
    if (wifiManager.isValid()) {
      auto wifiList =
          wifiManager.callObjectMethod("getScanResults", "()Ljava/util/List;");

      jint size = wifiList.callMethod<jint>("size", "()I");
      for (int i = 0; i < size; ++i) {
        auto scanResult =
            wifiList.callObjectMethod("get", "(I)Ljava/lang/Object;", i);
        auto scanInfo =
            ScanInfo{scanResult.getObjectField<jstring>("BSSID").toString(),
                     scanResult.getObjectField<jstring>("SSID").toString(),
                     0, // scanResult.getField<jint>("timestamp"),
                     scanResult.getField<jint>("frequency"),
                     static_cast<float>(scanResult.getField<jint>("level")),
                     0};
        scanInfos.append(scanInfo);
      }
    }
  }
  return scanInfos;
}

void AndroidScan::onData() {
  QtAndroid::androidActivity().callMethod<void>("unregisterBroadcastReceiver");
  mScanning = false;

  auto res = results();
  if (res.size() > 0) {
    emit scanFinished(res);
  } else {
    emit scanFailed(254);
  }
}

bool AndroidScan::measure(QPoint pos) {
  if (mScanning)
    return false;
  if (!(checkPermission("CHANGE_WIFI_STATE") &&
        checkPermission("ACCESS_FINE_LOCATION"))) {
    return false;
  }

  auto service_name = QAndroidJniObject::getStaticObjectField<jstring>(
      "android/content/Context", "WIFI_SERVICE");
  if (service_name.isValid()) {
    auto wifiManager = QtAndroid::androidActivity().callObjectMethod(
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
        service_name.object<jstring>());
    if (wifiManager.isValid()) {
      auto res = wifiManager.callMethod<jboolean>("startScan", "()Z");
      if (!res)
        return false;
      QtAndroid::androidActivity().callMethod<void>(
          "registerBroadcastReceiver");
      mScanning = true;
      emit scanStarted(pos);
      return true;
    }
  }
  return false;
}
