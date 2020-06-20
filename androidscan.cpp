#include "androidscan.h"
#include "androidhelper.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#include <functional>

AndroidScan::AndroidScan(QObject *parent) : QObject(parent) {
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

QVector<MeasurementEntry> AndroidScan::results() {
  auto scanInfos = QVector<MeasurementEntry>{};

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
        auto scanInfo = MeasurementEntry{
            Bss{scanResult.getObjectField<jstring>("BSSID").toString(),
                scanResult.getObjectField<jstring>("SSID").toString(),
                scanResult.getField<jint>("frequency"), 0},
            WiFiSignal,
            static_cast<double>(scanResult.getField<jint>("level"))};
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
    emit scanFailed(254, "No scan results.");
  }
}

std::optional<MeasurementEntry> AndroidScan::connected() {
  if (!(checkPermission("CHANGE_WIFI_STATE") &&
        checkPermission("ACCESS_FINE_LOCATION"))) {
    emit scanFailed(253, "no permission");
    return {};
  }

  auto service_name = QAndroidJniObject::getStaticObjectField<jstring>(
      "android/content/Context", "WIFI_SERVICE");
  if (service_name.isValid()) {
    auto wifiManager = QtAndroid::androidActivity().callObjectMethod(
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
        service_name.object<jstring>());
    if (wifiManager.isValid()) {
      auto isWifiEnabled =
          wifiManager.callMethod<jboolean>("isWifiEnabled", "()Z");
      if (!isWifiEnabled) {
        emit scanFailed(252, "wifi not enabled");
        return {};
      }

      auto wifiInfo = wifiManager.callObjectMethod(
          "getConnectionInfo", "()Landroid/net/wifi/WifiInfo;");
      if (wifiInfo.isValid()) {
        auto bssid =
            wifiInfo.callObjectMethod("getBSSID", "()Ljava/lang/String;");
        if (bssid.isValid()) {
          auto ssid =
              wifiInfo.callObjectMethod("getSSID", "()Ljava/lang/String;")
                  .toString();
          auto freq = wifiInfo.callMethod<jint>("getFrequency", "()I");
          auto level =
              static_cast<double>(wifiInfo.callMethod<jint>("getRssi", "()I"));

          if (ssid.startsWith("\"")) {
            ssid = ssid.remove(0, 1);
          }
          if (ssid.endsWith("\"")) {
            ssid = ssid.remove(ssid.length() - 1, 1);
          }

          auto scanInfo = MeasurementEntry{Bss{bssid.toString(), ssid, freq, 0},
                                           WiFiSignal, level};

          emit scanFinished({scanInfo});
          return scanInfo;
        } else {
          emit scanFailed(251, "not connected");
          return {};
        }
      }
    }
  }
  emit scanFailed(254, "faild");
  return {};
}

bool AndroidScan::measure() {
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
      return true;
    }
  }
  return false;
}
